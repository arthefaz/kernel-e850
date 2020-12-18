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
						   unsigned long size)
{
	struct samsung_dma_buffer *buffer;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	buffer->sg_table = kzalloc(sizeof(*buffer->sg_table), GFP_KERNEL);
	if (!buffer->sg_table)
		goto free_table;

	if (sg_alloc_table(buffer->sg_table, 1, GFP_KERNEL))
		goto free_sg;

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->heap = samsung_dma_heap;
	buffer->len = size;

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

int samsung_heap_add(struct samsung_dma_heap *samsung_dma_heap,
		     const char *name, const struct dma_heap_ops *ops)
{
	struct dma_heap_export_info exp_info;

	exp_info.name = name;
	exp_info.ops = ops;
	exp_info.priv = samsung_dma_heap;

	samsung_dma_heap->dma_heap = dma_heap_add(&exp_info);
	if (IS_ERR(samsung_dma_heap->dma_heap))
		return PTR_ERR(samsung_dma_heap->dma_heap);

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

int samsung_heap_create(struct device *dev, void *priv,
			void (*release)(struct samsung_dma_buffer *buffer), const char *name,
			const struct dma_heap_ops *ops)

{
	struct samsung_dma_heap *heap;
	int i, ret;

	heap = samsung_heap_init(dev, priv, release);
	if (IS_ERR(heap))
		return PTR_ERR(heap);

	return samsung_heap_add(heap, name, ops);
}

struct dma_buf *samsung_export_dmabuf(struct samsung_dma_buffer *buffer, unsigned long fd_flags)
{
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	exp_info.ops = &samsung_dma_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;

	return dma_buf_export(&exp_info);
}

static int __init samsung_dma_heap_init(void)
{
	int ret;

	ret = cma_dma_heap_init();
	if (ret)
		return ret;

	ret = carveout_dma_heap_init();
	if (ret) {
		cma_dma_heap_exit();

		return ret;
	}

	return 0;
}

static void __exit samsung_dma_heap_exit(void)
{
	carveout_dma_heap_exit();
	cma_dma_heap_exit();
}

module_init(samsung_dma_heap_init);
module_exit(samsung_dma_heap_exit);
MODULE_DESCRIPTION("DMA-BUF Samsung Heap");
MODULE_LICENSE("GPL v2");
