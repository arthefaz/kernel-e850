// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Heap Allocator - Common implementation
 *
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
 * Author: <hyesoo.yu@samsung.com> for Samsung
 */

#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/of.h>

#include "heap_private.h"

void heap_cache_flush(struct samsung_dma_buffer *buffer)
{
	struct device *dev = dma_heap_get_dev(buffer->heap->dma_heap);

	if (!dma_heap_skip_cached(buffer->flags))
		return;

	/*
	 * Flushing caches on buffer allocation is intended for preventing
	 * corruption from writing back to DRAM from the dirty cache lines
	 * while updating the buffer from DMA. However, cache flush should be
	 * performed on the entire allocated area if the buffer is to be
	 * protected from non-secure access to prevent the dirty write-back
	 * to the protected area.
	 */
	dma_map_sgtable(dev, buffer->sg_table, DMA_TO_DEVICE, 0);
	dma_unmap_sgtable(dev, buffer->sg_table, DMA_FROM_DEVICE, 0);
}

/*
 * It should be called by physically contiguous buffer.
 */
void heap_page_clean(struct page *pages, unsigned long size)
{
	unsigned long nr_pages, i;

	if (!PageHighMem(pages)) {
		memset(page_address(pages), 0, size);
		return;
	}

	nr_pages = PAGE_ALIGN(size) >> PAGE_SHIFT;

	for (i = 0; i < nr_pages; i++) {
		void *vaddr = kmap_atomic(&pages[i]);

		memset(vaddr, 0, PAGE_SIZE);
		kunmap_atomic(vaddr);
	}
}

struct samsung_dma_buffer *samsung_dma_buffer_init(struct samsung_dma_heap *samsung_dma_heap,
						   unsigned long size, unsigned int nents)
{
	struct samsung_dma_buffer *buffer;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	buffer->sg_table = kzalloc(sizeof(*buffer->sg_table), GFP_KERNEL);
	if (!buffer->sg_table)
		goto free_table;

	if (sg_alloc_table(buffer->sg_table, nents, GFP_KERNEL))
		goto free_sg;

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->heap = samsung_dma_heap;
	buffer->len = size;
	buffer->flags = samsung_dma_heap->flags;

	return buffer;
free_sg:
	kfree(buffer->sg_table);
free_table:
	kfree(buffer);

	return ERR_PTR(-ENOMEM);
}

void samsung_dma_buffer_remove(struct samsung_dma_buffer *buffer)
{
	sg_free_table(buffer->sg_table);
	kfree(buffer->sg_table);
	kfree(buffer);
}

const char *samsung_add_heap_name(unsigned long flags)
{
	if (dma_heap_flags_uncached(flags))
		return "-uncached";

	if (dma_heap_flags_protected(flags))
		return "-secure";

	return "";
}

int samsung_heap_add(struct device *dev, struct samsung_dma_heap *samsung_dma_heap,
		     const char *name, const struct dma_heap_ops *ops)
{
	struct dma_heap_export_info exp_info;
	char *heap_name;

	heap_name = devm_kasprintf(dev, GFP_KERNEL, "%s%s", name,
				   samsung_add_heap_name(samsung_dma_heap->flags));
	if (!heap_name)
		return -ENOMEM;

	exp_info.name = heap_name;
	exp_info.ops = ops;
	exp_info.priv = samsung_dma_heap;

	samsung_dma_heap->name = heap_name;
	samsung_dma_heap->dma_heap = dma_heap_add(&exp_info);
	if (IS_ERR(samsung_dma_heap->dma_heap))
		return PTR_ERR(samsung_dma_heap->dma_heap);

	dma_coerce_mask_and_coherent(dma_heap_get_dev(samsung_dma_heap->dma_heap),
				     DMA_BIT_MASK(36));

	return 0;
}

struct samsung_dma_heap *samsung_heap_init(struct device *dev, void *priv,
					   void (*release)(struct samsung_dma_buffer *buffer))
{
	struct samsung_dma_heap *samsung_dma_heap;
	unsigned int alignment = PAGE_SIZE, order;

	samsung_dma_heap = devm_kzalloc(dev, sizeof(*samsung_dma_heap), GFP_KERNEL);
	if (!samsung_dma_heap)
		return ERR_PTR(-ENOMEM);

	of_property_read_u32(dev->of_node, "dma_heap,alignment", &alignment);
	order = min_t(unsigned int, get_order(alignment), MAX_ORDER);

	samsung_dma_heap->alignment = 1 << (order + PAGE_SHIFT);
	samsung_dma_heap->release = release;
	samsung_dma_heap->priv = priv;

	return samsung_dma_heap;
}

static const unsigned int cachable_heap_type[] = {
	0,
	DMA_HEAP_FLAG_UNCACHED,
};
#define num_cachable_heaps ARRAY_SIZE(cachable_heap_type)

static const unsigned int prot_heap_type[] = {
	DMA_HEAP_FLAG_PROTECTED,
};
#define num_prot_heaps ARRAY_SIZE(prot_heap_type)

/*
 * Maximum heap types is defined by cachable heap types
 * because prot heap type is always only 1.
 */
#define num_max_heaps (num_cachable_heaps)

/*
 * NOTE: samsung_heap_create returns error when heap creation fails.
 * In case of -ENODEV, it means that the secure heap doesn't need to be added
 * if system doesn't support content protection.
 */
int samsung_heap_create(struct device *dev, void *priv,
			void (*release)(struct samsung_dma_buffer *buffer),
			const struct dma_heap_ops *ops)

{
	struct samsung_dma_heap *heap[num_max_heaps];
	const unsigned int *types;
	int i, ret, count, protid = 0;
	const char *name;
	bool video_aligned_heap;

	if (of_property_read_string(dev->of_node, "dma_heap,name", &name)) {
		perrfn("The heap should define name on device node");
		return -EINVAL;
	}
	/*
	 * Secure heap should allocate only secure buffer.
	 * Normal cachable heap and uncachable heaps are not registered.
	 */
	if (of_property_read_bool(dev->of_node, "dma_heap,secure")) {
		of_property_read_u32(dev->of_node, "dma_heap,protection_id", &protid);
		if (!protid) {
			perrfn("Secure heap should be set with protection id");
			return -EINVAL;
		}

		if (!IS_ENABLED(CONFIG_EXYNOS_CONTENT_PATH_PROTECTION))
			return -ENODEV;

		count = num_prot_heaps;
		types = prot_heap_type;
	} else {
		count = num_cachable_heaps;
		types = cachable_heap_type;
	}

	video_aligned_heap = of_property_read_bool(dev->of_node, "dma_heap,video_aligned");

	for (i = 0; i < count; i++) {
		heap[i] = samsung_heap_init(dev, priv, release);
		if (IS_ERR(heap[i])) {
			ret = PTR_ERR(heap[i]);
			goto heap_put;
		}

		heap[i]->flags = types[i];
		heap[i]->protection_id = protid;

		if (video_aligned_heap)
			heap[i]->flags |= DMA_HEAP_FLAG_VIDEO_ALIGNED;

		ret = samsung_heap_add(dev, heap[i], name, ops);
		if (ret)
			goto heap_put;
	}

	return 0;
heap_put:
	while (i-- > 0)
		dma_heap_put(heap[i]->dma_heap);

	return ret;
}

struct dma_buf *samsung_export_dmabuf(struct samsung_dma_buffer *buffer, unsigned long fd_flags)
{
	DEFINE_SAMSUNG_DMA_BUF_EXPORT_INFO(exp_info, buffer->heap->name);

	exp_info.ops = &samsung_dma_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;

	return dma_buf_export(&exp_info);
}

static int __init samsung_dma_heap_init(void)
{
	int ret;

	ret = secure_iova_pool_create();
	if (ret)
		return ret;

	ret = cma_dma_heap_init();
	if (ret)
		goto err_cma;

	ret = carveout_dma_heap_init();
	if (ret)
		goto err_carveout;

	ret = system_dma_heap_init();
	if (ret)
		goto err_system;

	return 0;
err_system:
	carveout_dma_heap_exit();
err_carveout:
	cma_dma_heap_exit();
err_cma:
	secure_iova_pool_destroy();

	return ret;
}

static void __exit samsung_dma_heap_exit(void)
{
	system_dma_heap_exit();
	carveout_dma_heap_exit();
	cma_dma_heap_exit();

	secure_iova_pool_destroy();
}

module_init(samsung_dma_heap_init);
module_exit(samsung_dma_heap_exit);
MODULE_DESCRIPTION("DMA-BUF Samsung Heap");
MODULE_LICENSE("GPL v2");
