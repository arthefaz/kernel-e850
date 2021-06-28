// SPDX-License-Identifier: GPL-2.0
/*
 * dma-buf-trace to track the relationship between dmabuf and process.
 *
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
 */

#include <linux/anon_inodes.h>
#include <linux/device.h>
#include <linux/dma-buf.h>
#include <linux/fdtable.h>
#include <linux/slab.h>
#include <linux/mm_types.h>
#include <linux/miscdevice.h>
#include <linux/ratelimit.h>
#include <linux/spinlock.h>
#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#include <linux/types.h>
#include <linux/sort.h>
#include <trace/hooks/mm.h>

#include "heap_private.h"

struct dmabuf_trace_ref {
	struct list_head task_node;
	struct list_head buffer_node;
	struct dmabuf_trace_task *task;
	struct dmabuf_trace_buffer *buffer;
	int refcount;
};

struct dmabuf_trace_task {
	struct list_head node;
	struct list_head ref_list;

	struct task_struct *task;
	struct file *file;
	struct dentry *debug_task;

	int mmap_count;
	size_t mmap_size;
};

struct dmabuf_trace_buffer {
	struct list_head node;
	struct list_head ref_list;

	struct dma_buf *dmabuf;
	int shared_count;
};

static struct list_head buffer_list = LIST_HEAD_INIT(buffer_list);
static unsigned long dmabuf_trace_buffer_size;
static unsigned long dmabuf_trace_buffer_num;

/*
 * head_task.node is the head node of all other dmabuf_trace_task.node.
 * At the same time, head_task itself maintains the buffer information allocated
 * by the kernel threads.
 */
static struct dmabuf_trace_task head_task;
static DEFINE_MUTEX(trace_lock);

#define INIT_NUM_SORTED_ARRAY 1024

static size_t *sorted_array;
static unsigned long num_sorted_array = INIT_NUM_SORTED_ARRAY;

#define MAX_ATTACHED_DEVICE 64
static struct dmabuf_trace_device {
	struct device *dev;
	size_t size; /* the total size of referenced buffer */
	unsigned long refcnt; /* the number of referenced buffer */
	unsigned long mapcnt; /* the number of total dma_buf_map_attachment count */
} attach_lists[MAX_ATTACHED_DEVICE];
static unsigned int num_devices;

static struct dmabuf_trace_device *dmabuf_trace_get_device(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < num_devices; i++) {
		if (attach_lists[i].dev == dev)
			return &attach_lists[i];
	}

	BUG_ON(num_devices == MAX_ATTACHED_DEVICE);
	attach_lists[num_devices].dev = dev;

	return &attach_lists[num_devices++];
}

void dmabuf_trace_map(struct dma_buf *dmabuf, struct dma_iovm_map *iovm_map)
{
	struct dmabuf_trace_device *trace_device;

	mutex_lock(&trace_lock);
	trace_device = dmabuf_trace_get_device(iovm_map->dev);

	if (iovm_map->mapcnt == 1) {
		trace_device->size += dmabuf->size;
		trace_device->refcnt++;
	}

	trace_device->mapcnt++;
	mutex_unlock(&trace_lock);
}

void dmabuf_trace_unmap(struct dma_buf *dmabuf, struct dma_iovm_map *iovm_map, struct device *dev)
{
	struct dmabuf_trace_device *trace_device;

	mutex_lock(&trace_lock);
	trace_device = dmabuf_trace_get_device(dev);

	if (!iovm_map || iovm_map->mapcnt == 0) {
		trace_device->size -= dmabuf->size;
		trace_device->refcnt--;
	}

	trace_device->mapcnt--;
	mutex_unlock(&trace_lock);
}

struct dmabuf_fd_iterdata {
	int fd_ref_size;
	int fd_ref_cnt;
};

int dmabuf_traverse_filetable(const void *t, struct file *file, unsigned fd)
{
	struct dmabuf_fd_iterdata *iterdata = (struct dmabuf_fd_iterdata*)t;
	struct dma_buf *dmabuf;

	if (!is_dma_buf_file(file))
		return 0;

	dmabuf = file->private_data;

	iterdata->fd_ref_cnt++;
	iterdata->fd_ref_size += dmabuf->size >> 10;

	return 0;
}

static int dmabuf_trace_buffer_size_compare(const void *p1, const void *p2)
{
	if (*((size_t *)p1) > (*((size_t *)p2)))
		return 1;
	else if (*((size_t *)p1) < (*((size_t *)p2)))
		return -1;
	return 0;
}

void show_dmabuf_trace_info(void)
{
	struct dmabuf_trace_task *task = &head_task;
	struct dmabuf_trace_buffer *buffer;
	int i, count, num_skipped_buffer = 0, num_buffer = 0;
	size_t size;

	pr_info("\nDma-buf Info:\n");
	/*
	 *            comm   pid   fdrefcnt mmaprefcnt fdsize(kb) mmapsize(kb)
	 * composer@2.4-se   552         26         26     104736     104736
	 *  surfaceflinger   636         40         40     110296     110296
	 */
	pr_info("%20s %5s %10s %10s %10s %10s\n",
		"comm", "pid", "fdrefcnt", "mmaprefcnt", "fdsize(kb)", "mmapsize(kb)");

	if (!mutex_trylock(&trace_lock))
		return;

	task = list_next_entry(task, node);
	do {
		struct dmabuf_fd_iterdata iterdata;
		bool skip = false;

		iterdata.fd_ref_cnt = iterdata.fd_ref_size = 0;

		if (spin_trylock(&task->task->alloc_lock)) {
			iterate_fd(task->task->files, 0, dmabuf_traverse_filetable, &iterdata);
			task_unlock(task->task);
		} else {
			iterdata.fd_ref_cnt = iterdata.fd_ref_size = 0;
			skip = true;
		}

		pr_info("%20s %5d %10d %10d %10zu %10zu %s\n",
			task->task->comm, task->task->pid, iterdata.fd_ref_cnt, task->mmap_count,
			iterdata.fd_ref_size, task->mmap_size / 1024,
			skip ? "-> skip by lock contention" : "");
		task = list_next_entry(task, node);
	} while (&task->node != &head_task.node);

	pr_info("Attached device list:\n");
	pr_info("%20s %20s %10s %10s\n", "attached device", "devsize(kb)", "devrefcnt", "mapcount");

	for (i = 0; i < num_devices; i++)
		pr_info("%20s %20zu %10lu %10lu\n",
			dev_name(attach_lists[i].dev), attach_lists[i].size / 1024,
			attach_lists[i].refcnt, attach_lists[i].mapcnt);

	list_for_each_entry(buffer, &buffer_list, node) {
		sorted_array[num_buffer++] = buffer->dmabuf->size;
		if (num_buffer == num_sorted_array) {
			num_skipped_buffer = dmabuf_trace_buffer_num - num_sorted_array;
			break;
		}
	}
	mutex_unlock(&trace_lock);

	if (!num_buffer)
		return;

	sort(sorted_array, num_buffer, sizeof(*sorted_array), dmabuf_trace_buffer_size_compare, NULL);

	pr_info("Dma-buf size statistics: (skipped bufcnt %d)\n", num_skipped_buffer);
	pr_info("%10s : %8s\n", "size(kb)", "count");

	size = sorted_array[0];
	count = 1;
	for (i = 1; i < num_buffer; i++) {
		if (size == sorted_array[i]) {
			count++;
			continue;
		}
		pr_info("%10zu : %8d\n", size / 1024, count);
		size = sorted_array[i];
		count = 1;
	}
	pr_info("%10zu : %8d\n", size / 1024, count);
}

void show_dmabuf_dva(struct device *dev)
{
	struct dmabuf_trace_buffer *buffer;
	struct iommu_domain *domain = iommu_get_domain_for_dev(dev);
	size_t mapped_size = 0;
	int cnt = 0;

	if (!dev_iommu_fwspec_get(dev))
		return;

	mutex_lock(&trace_lock);
	pr_info("The %s domain device virtual address list: (kbsize@base)\n", dev_name(dev));

	list_for_each_entry(buffer, &buffer_list, node) {
		struct dma_buf *dmabuf = buffer->dmabuf;
		struct samsung_dma_buffer *samsung_dma_buffer = dmabuf->priv;
		struct dma_iovm_map *iovm_map;

		mutex_lock(&samsung_dma_buffer->lock);
		list_for_each_entry(iovm_map, &samsung_dma_buffer->attachments, list) {
			if (domain != iommu_get_domain_for_dev(iovm_map->dev))
				continue;
			mapped_size += dmabuf->size;

			pr_cont("%8zu@%#5lx ", dmabuf->size >> 10,
				sg_dma_address(iovm_map->table.sgl) >> 12);
			if (cnt++ & 16) {
				pr_cont("\n");
				cnt = 0;
			}
		}
		mutex_unlock(&samsung_dma_buffer->lock);
	}
	pr_info("Total dva allocated size %zukb\n", mapped_size >> 10);
	mutex_unlock(&trace_lock);
}

static void show_dmabuf_trace_handler(void *data, unsigned int filter, nodemask_t *nodemask)
{
	static DEFINE_RATELIMIT_STATE(dmabuf_trace_ratelimit, HZ * 10, 1);

	if (!__ratelimit(&dmabuf_trace_ratelimit))
		return;

	show_dmabuf_trace_info();
}

static void dmabuf_trace_free_ref_force(struct dmabuf_trace_ref *ref)
{
	ref->buffer->shared_count--;

	ref->task->mmap_size -= ref->buffer->dmabuf->size;
	ref->task->mmap_count--;

	list_del(&ref->buffer_node);
	list_del(&ref->task_node);

	kfree(ref);
}

static int dmabuf_trace_free_ref(struct dmabuf_trace_ref *ref)
{
	/* The reference has never been registered */
	if (WARN_ON(ref->refcount == 0))
		return -EINVAL;

	if (--ref->refcount == 0)
		dmabuf_trace_free_ref_force(ref);

	return 0;
}

static int dmabuf_trace_task_release(struct inode *inode, struct file *file)
{
	struct dmabuf_trace_task *task = file->private_data;
	struct dmabuf_trace_ref *ref, *tmp;

	if (!(task->task->flags & PF_EXITING)) {
		pr_err("%s: Invalid to close '%d' on process '%s'(%x, %lx)\n",
		       __func__, task->task->pid, task->task->comm,
		       task->task->flags, task->task->state);
		dump_stack();
	}

	put_task_struct(task->task);

	mutex_lock(&trace_lock);
	list_for_each_entry_safe(ref, tmp, &task->ref_list, task_node)
		dmabuf_trace_free_ref_force(ref);
	list_del(&task->node);
	mutex_unlock(&trace_lock);

	kfree(task);

	return 0;
}

static const struct file_operations dmabuf_trace_task_fops = {
	.release = dmabuf_trace_task_release,
};

static struct dmabuf_trace_buffer *dmabuf_trace_get_buffer(
		struct dma_buf *dmabuf)
{
	struct dmabuf_trace_buffer *buffer;

	list_for_each_entry(buffer, &buffer_list, node)
		if (buffer->dmabuf == dmabuf)
			return buffer;

	return NULL;
}

static struct dmabuf_trace_task *dmabuf_trace_get_task_noalloc(void)
{
	struct dmabuf_trace_task *task;

	if (!current->mm && (current->flags & PF_KTHREAD))
		return &head_task;

	/*
	 * init process, pid 1 closes file descriptor after booting.
	 * At that time, the trace buffers of init process are released,
	 * so we use head task to track the buffer of init process instead of
	 * creating dmabuf_trace_task for init process.
	 */
	if (current->group_leader->pid == 1)
		return &head_task;

	list_for_each_entry(task, &head_task.node, node)
		if (task->task == current->group_leader)
			return task;

	return NULL;
}

static struct dmabuf_trace_task *dmabuf_trace_get_task(void)
{
	struct dmabuf_trace_task *task;
	unsigned char name[10];
	int ret, fd;

	task = dmabuf_trace_get_task_noalloc();
	if (task)
		return task;

	task = kzalloc(sizeof(*task), GFP_KERNEL);
	if (!task)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&task->node);
	INIT_LIST_HEAD(&task->ref_list);

	scnprintf(name, 10, "%d", current->group_leader->pid);

	get_task_struct(current->group_leader);
	task->task = current->group_leader;

	ret = get_unused_fd_flags(O_RDONLY | O_CLOEXEC);
	if (ret < 0)
		goto err_fd;
	fd = ret;

	task->file = anon_inode_getfile(name, &dmabuf_trace_task_fops, task, O_RDWR);
	if (IS_ERR(task->file)) {
		ret = PTR_ERR(task->file);
		goto err_inode;
	}

	fd_install(fd, task->file);

	list_add_tail(&task->node, &head_task.node);

	return task;

err_inode:
	put_unused_fd(fd);
err_fd:
	put_task_struct(current->group_leader);
	kfree(task);

	pr_err("%s: Failed to get task(err %d)\n", __func__, ret);

	return ERR_PTR(ret);
}

static struct dmabuf_trace_ref *dmabuf_trace_get_ref_noalloc(struct dmabuf_trace_buffer *buffer,
							     struct dmabuf_trace_task *task)
{
	struct dmabuf_trace_ref *ref;

	list_for_each_entry(ref, &task->ref_list, task_node)
		if (ref->buffer == buffer)
			return ref;

	return NULL;
}

static struct dmabuf_trace_ref *dmabuf_trace_get_ref(struct dmabuf_trace_buffer *buffer,
						     struct dmabuf_trace_task *task)
{
	struct dmabuf_trace_ref *ref;

	ref = dmabuf_trace_get_ref_noalloc(buffer, task);
	if (ref) {
		ref->refcount++;
		return ref;
	}

	ref = kzalloc(sizeof(*ref), GFP_KERNEL);
	if (!ref)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&ref->buffer_node);
	INIT_LIST_HEAD(&ref->task_node);

	ref->task = task;
	ref->buffer = buffer;
	ref->refcount = 1;

	list_add_tail(&ref->task_node, &task->ref_list);
	list_add_tail(&ref->buffer_node, &buffer->ref_list);

	task->mmap_count++;
	ref->task->mmap_size += ref->buffer->dmabuf->size;

	buffer->shared_count++;

	return ref;
}

/**
 * dmabuf_trace_alloc - get reference after creating dmabuf.
 * @dmabuf : buffer to register reference.
 *
 * This create a ref that has relationship between dmabuf
 * and process that requested allocation, and also create
 * the buffer object to trace.
 */
int dmabuf_trace_alloc(struct dma_buf *dmabuf)
{
	struct dmabuf_trace_buffer *buffer;
	struct dmabuf_trace_task *task;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	INIT_LIST_HEAD(&buffer->ref_list);
	buffer->dmabuf = dmabuf;

	mutex_lock(&trace_lock);
	list_add_tail(&buffer->node, &buffer_list);
	dmabuf_trace_buffer_size += buffer->dmabuf->size;
	dmabuf_trace_buffer_num++;

	if (dmabuf_trace_buffer_num >= num_sorted_array) {
		size_t *realloced_array = kvmalloc_array(num_sorted_array * 2,
							 sizeof(*sorted_array), GFP_KERNEL);
		/*
		 * If reallocation is failed, it is not considered an error.
		 * This failure causes just some information to be missing from debug logs.
		 */
		if (realloced_array) {
			kvfree(sorted_array);
			sorted_array = realloced_array;
			num_sorted_array *= 2;
		}
	}

	task = dmabuf_trace_get_task();
	if (IS_ERR(task)) {
		mutex_unlock(&trace_lock);
		return PTR_ERR(task);
	}
	mutex_unlock(&trace_lock);

	return 0;
}

/**
 * dmabuf_trace_free - release references after removing buffer.
 * @dmabuf : buffer to release reference.
 *
 * This remove refs that connected with released dmabuf.
 */
void dmabuf_trace_free(struct dma_buf *dmabuf)
{
	struct dmabuf_trace_buffer *buffer;
	struct dmabuf_trace_ref *ref, *tmp;

	mutex_lock(&trace_lock);
	buffer = dmabuf_trace_get_buffer(dmabuf);
	if (!buffer) {
		mutex_unlock(&trace_lock);
		return;
	}

	list_for_each_entry_safe(ref, tmp, &buffer->ref_list, buffer_node)
		dmabuf_trace_free_ref_force(ref);

	list_del(&buffer->node);
	dmabuf_trace_buffer_size -= buffer->dmabuf->size;
	dmabuf_trace_buffer_num--;
	mutex_unlock(&trace_lock);
	kfree(buffer);
}

/**
 * dmabuf_trace_register - create ref between task and buffer.
 * @dmabuf : buffer to register reference.
 *
 * This create ref between current task and buffer.
 */
int dmabuf_trace_track_buffer(struct dma_buf *dmabuf)
{
	struct dmabuf_trace_buffer *buffer;
	struct dmabuf_trace_task *task;
	struct dmabuf_trace_ref *ref;

	mutex_lock(&trace_lock);
	task = dmabuf_trace_get_task();
	if (IS_ERR(task)) {
		mutex_unlock(&trace_lock);
		return PTR_ERR(task);
	}

	buffer = dmabuf_trace_get_buffer(dmabuf);
	if (!buffer) {
		mutex_unlock(&trace_lock);
		return -ENOENT;
	}

	ref = dmabuf_trace_get_ref(buffer, task);
	if (IS_ERR(ref)) {
		mutex_unlock(&trace_lock);
		return PTR_ERR(ref);
	}

	mutex_unlock(&trace_lock);
	return 0;
}

/**
 * dmabuf_trace_unregister - remove ref between task and buffer.
 * @dmabuf : buffer to unregister reference.
 *
 * This remove ref between current task and buffer.
 */
int dmabuf_trace_untrack_buffer(struct dma_buf *dmabuf)
{
	struct dmabuf_trace_buffer *buffer;
	struct dmabuf_trace_task *task;
	struct dmabuf_trace_ref *ref;
	int ret;

	mutex_lock(&trace_lock);
	task = dmabuf_trace_get_task_noalloc();
	if (!task) {
		mutex_unlock(&trace_lock);
		return -ESRCH;
	}

	buffer = dmabuf_trace_get_buffer(dmabuf);
	if (!buffer) {
		mutex_unlock(&trace_lock);
		return -ENOENT;
	}

	ref = dmabuf_trace_get_ref_noalloc(buffer, task);
	if (!ref) {
		mutex_unlock(&trace_lock);
		return -ENOENT;
	}

	ret = dmabuf_trace_free_ref(ref);
	mutex_unlock(&trace_lock);

	return 0;
}

int __init dmabuf_trace_create(void)
{
	INIT_LIST_HEAD(&head_task.node);
	INIT_LIST_HEAD(&head_task.ref_list);

	register_trace_android_vh_show_mem(show_dmabuf_trace_handler, NULL);

	sorted_array = kvmalloc_array(INIT_NUM_SORTED_ARRAY, sizeof(*sorted_array), GFP_KERNEL);
	if (!sorted_array) {
		num_sorted_array = 0;
		return -ENOMEM;
	}

	pr_info("Initialized dma-buf trace successfully.\n");

	return 0;
}

void dmabuf_trace_remove(void)
{
	kvfree(sorted_array);
	num_sorted_array = 0;
}
