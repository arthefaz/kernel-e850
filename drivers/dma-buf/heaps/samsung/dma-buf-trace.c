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
static unsigned long buffer_size;

/*
 * head_task.node is the head node of all other dmabuf_trace_task.node.
 * At the same time, head_task itself maintains the buffer information allocated
 * by the kernel threads.
 */
static struct dmabuf_trace_task head_task;
static DEFINE_MUTEX(trace_lock);

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

void show_dmabuf_trace_info(void)
{
	struct dmabuf_trace_task *task = &head_task;
	struct dmabuf_trace_buffer *buffer;

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

		iterdata.fd_ref_cnt = iterdata.fd_ref_size = 0;

		task_lock(task->task);
		iterate_fd(task->task->files, 0, dmabuf_traverse_filetable, &iterdata);
		task_unlock(task->task);

		pr_info("%20s %5d %10d %10d %10zu %10zu\n",
			task->task->comm, task->task->pid, iterdata.fd_ref_cnt, task->mmap_count,
			iterdata.fd_ref_size, task->mmap_size / 1024);
		task = list_next_entry(task, node);

	} while (&task->node != &head_task.node);

	/*
	 * Show buffer list only if there is a possibility of dma-heap leakage.
	 * If 1/4 of total RAM size is filled with dma-heap buffer, we suspect
	 * a dma-heap leakage.
	 */
	if (buffer_size >> PAGE_SHIFT < totalram_pages() / 4) {
		mutex_unlock(&trace_lock);
		return;
	}

	/*
	 *  node                  exp       size   refcount mmaprefcnt devrefcnt attached device..
	 * 36561      system-uncached     638976          3          1         1 18500000.mali(1)
	 * 36562               system      40960          2          1         0
	 */
	pr_info("\n%8s %20s %10s %10s %10s %10s %s\n",
		"inode", "exp", "size", "refcount", "mmaprefcnt", "devrefcnt", "attached device..");

	list_for_each_entry(buffer, &buffer_list, node) {
		struct dma_buf *dmabuf = buffer->dmabuf;
		struct samsung_dma_buffer *samsung_dma_buffer = dmabuf->priv;
		struct dma_iovm_map *iovm_map;
		int mapcnt = 0;

		pr_cont("%8lu %20s %10zu %10d %10d ", file_inode(dmabuf->file)->i_ino,
			dmabuf->exp_name, dmabuf->size, file_count(dmabuf->file),
			buffer->shared_count);

		mutex_lock(&samsung_dma_buffer->lock);
		list_for_each_entry(iovm_map, &samsung_dma_buffer->attachments, list)
			mapcnt += iovm_map->mapcnt;

		pr_cont("%10d ", mapcnt);
		list_for_each_entry(iovm_map, &samsung_dma_buffer->attachments, list)
			pr_cont("%s(%d) ", dev_name(iovm_map->dev), iovm_map->mapcnt);

		pr_cont("\n");
		mutex_unlock(&samsung_dma_buffer->lock);
	}
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
	buffer_size += buffer->dmabuf->size;

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
	buffer_size -= buffer->dmabuf->size;
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

void __init dmabuf_trace_create(void)
{
	INIT_LIST_HEAD(&head_task.node);
	INIT_LIST_HEAD(&head_task.ref_list);

	register_trace_android_vh_show_mem(show_dmabuf_trace_handler, NULL);

	pr_info("Initialized dma-buf trace successfully.\n");
}
