// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF System heap exporter for Samsung
 *
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
 * Author: <hyesoo.yu@samsung.com> for Samsung
 */

#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/dma-heap.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "heap_private.h"

#define HIGH_ORDER_GFP  (((GFP_HIGHUSER | __GFP_ZERO | __GFP_NOWARN \
				| __GFP_NORETRY) & ~__GFP_RECLAIM) \
				| __GFP_COMP)
#define LOW_ORDER_GFP (GFP_HIGHUSER | __GFP_ZERO | __GFP_COMP)
static gfp_t order_flags[] = {HIGH_ORDER_GFP, HIGH_ORDER_GFP, HIGH_ORDER_GFP, LOW_ORDER_GFP};
/*
 * The selection of the orders used for allocation (2MB, 1MB, 64K, 4K) is designed
 * to match with the sizes often found in IOMMUs. Using high order pages instead
 * of order 0 pages can significantly improve the performance of many IOMMUs
 * by reducing TLB pressure and time spent updating page tables.
 */
static const unsigned int orders[] = {9, 8, 4, 0};
#define NUM_ORDERS ARRAY_SIZE(orders)

static struct page *alloc_largest_available(unsigned long size,
					    unsigned int max_order)
{
	struct page *page;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (size < (PAGE_SIZE << orders[i]))
			continue;
		if (max_order < orders[i])
			continue;

		page = alloc_pages(order_flags[i], orders[i]);
		if (!page)
			continue;
		return page;
	}
	return NULL;
}

static struct dma_buf *system_heap_allocate(struct dma_heap *heap, unsigned long len,
					    unsigned long fd_flags, unsigned long heap_flags)
{
	struct samsung_dma_heap *samsung_dma_heap = dma_heap_get_drvdata(heap);
	struct samsung_dma_buffer *buffer;
	struct scatterlist *sg;
	struct dma_buf *dmabuf;
	struct list_head pages;
	struct page *page, *tmp_page;
	unsigned long size_remaining = len;
	unsigned int max_order = orders[0];
	int i, ret = -ENOMEM;

	INIT_LIST_HEAD(&pages);
	i = 0;
	while (size_remaining > 0) {
		/*
		 * Avoid trying to allocate memory if the process
		 * has been killed by SIGKILL
		 */
		if (fatal_signal_pending(current)) {
			perrfn("Fatal signal pending pid #%d", current->pid);
			ret = -EINTR;
			goto free_buffer;
		}

		page = alloc_largest_available(size_remaining, max_order);
		if (!page)
			goto free_buffer;

		list_add_tail(&page->lru, &pages);
		size_remaining -= page_size(page);
		max_order = compound_order(page);
		i++;
	}

	buffer = samsung_dma_buffer_init(samsung_dma_heap, len, i);
	if (IS_ERR(buffer))
		goto free_buffer;

	sg = buffer->sg_table->sgl;
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		sg_set_page(sg, page, page_size(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}

	heap_cache_flush(buffer);

	dmabuf = samsung_export_dmabuf(buffer, fd_flags);
	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto free_export;
	}

	return dmabuf;

free_export:
	for_each_sgtable_sg(buffer->sg_table, sg, i) {
		struct page *p = sg_page(sg);

		__free_pages(p, compound_order(p));
	}
	samsung_dma_buffer_remove(buffer);
free_buffer:
	list_for_each_entry_safe(page, tmp_page, &pages, lru)
		__free_pages(page, compound_order(page));

	return ERR_PTR(ret);
}

static const struct dma_heap_ops system_heap_ops = {
	.allocate = system_heap_allocate,
};

static void system_heap_release(struct samsung_dma_buffer *buffer)
{
	struct scatterlist *sg;
	int i;

	for_each_sgtable_sg(buffer->sg_table, sg, i) {
		struct page *page = sg_page(sg);

		__free_pages(page, compound_order(page));
	}
	samsung_dma_buffer_remove(buffer);
}

struct device sysdev;

int __init system_dma_heap_init(void)
{
	int ret;

	device_initialize(&sysdev);

	ret = samsung_heap_create(&sysdev, NULL, system_heap_release, "system", &system_heap_ops);
	if (ret)
		return ret;

	pr_info("Register system dma-heap successfully\n");

	return 0;
}

void system_dma_heap_exit(void)
{
}
