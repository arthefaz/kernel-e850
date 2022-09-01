// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF HPA heap exporter for Samsung
 *
 * Copyright (C) 2021 Samsung Electronics Co., Ltd.
 * Author: <hyesoo.yu@samsung.com> for Samsung
 */

#include <linux/cma.h>
#include <linux/device.h>
#include <linux/dma-buf.h>
#include <linux/dma-direct.h>
#include <linux/dma-heap.h>
#include <linux/err.h>
#include <linux/genalloc.h>
#include <linux/gfp.h>
#include <linux/highmem.h>
#include <linux/iommu.h>
#include <linux/kmemleak.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/sort.h>

#include "secure_buffer.h"

struct hpa_dma_heap {
	struct dma_heap *dma_heap;
	const char *name;
	unsigned int protection_id;
};

struct hpa_dma_buffer {
	struct hpa_dma_heap *heap;
	struct sg_table sg_table;
	void *priv;
};

static struct hpa_dma_heap hpa_heaps[] = {
	{
		.name = "vframe-secure",
		.protection_id = 5,
	},
	{
		.name = "vscaler-secure",
		.protection_id = 6,
	},
	{
		.name = "system-secure-gpu_buffer-secure",
		.protection_id = 9,
	},
};

#define MAX_EXCEPTION_AREAS 4
static phys_addr_t hpa_exception_areas[MAX_EXCEPTION_AREAS][2];
static int nr_hpa_exception;

#define HPA_SECURE_DMA_BASE	0x80000000
#define HPA_SECURE_DMA_SIZE	0x80000000

#define HPA_DEFAULT_ORDER 4
#define HPA_CHUNK_SIZE  (PAGE_SIZE << HPA_DEFAULT_ORDER)
#define HPA_PAGE_COUNT(len) (ALIGN(len, HPA_CHUNK_SIZE) / HPA_CHUNK_SIZE)
#define HPA_MAX_CHUNK_COUNT ((PAGE_SIZE * 2) / sizeof(struct page *))

/*
 * Alignment to a secure address larger than 16MiB is not beneficial because
 * the protection alignment just needs 64KiB by the buffer protection H/W and
 * the largest granule of H/W security firewall (the secure context of SysMMU)
 * is 16MiB.
 */
#define MAX_SECURE_VA_ALIGN	(SZ_16M / PAGE_SIZE)

static struct gen_pool *hpa_iova_pool;

static int hpa_secure_iova_pool_init(void)
{
	int ret;

	hpa_iova_pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!hpa_iova_pool) {
		pr_err("failed to create HPA IOVA pool\n");
		return -ENOMEM;
	}

	ret = gen_pool_add(hpa_iova_pool, HPA_SECURE_DMA_BASE, HPA_SECURE_DMA_SIZE, -1);
	if (ret) {
		pr_err("failed to set address range of HPA IOVA pool\n");
		gen_pool_destroy(hpa_iova_pool);
		return ret;
	}

	return 0;
}

unsigned long hpa_iova_alloc(unsigned long size, unsigned int align)
{
	unsigned long out_addr;
	struct genpool_data_align alignment = {
		.align = max_t(int, PFN_DOWN(align), MAX_SECURE_VA_ALIGN),
	};

	if (WARN_ON_ONCE(!hpa_iova_pool))
		return 0;

	out_addr = gen_pool_alloc_algo(hpa_iova_pool, size, gen_pool_first_fit_align, &alignment);
	if (out_addr == 0)
		pr_err("failed alloc secure iova. %zu/%zu bytes used\n",
		       gen_pool_avail(hpa_iova_pool),
		       gen_pool_size(hpa_iova_pool));

	return out_addr;
}

static void hpa_iova_free(unsigned long addr, unsigned long size)
{
	gen_pool_free(hpa_iova_pool, addr, size);
}

static int hpa_secure_protect(struct buffer_prot_info *protdesc, struct device *dev)
{
	unsigned long size = protdesc->chunk_count * protdesc->chunk_size;
	unsigned long ret = 0, dma_addr = 0;

	dma_addr = hpa_iova_alloc(size, max_t(u32, HPA_CHUNK_SIZE, PAGE_SIZE));
	if (!dma_addr)
		return -ENOMEM;

	protdesc->dma_addr = (unsigned int)dma_addr;

	dma_map_single(dev, protdesc, sizeof(*protdesc), DMA_TO_DEVICE);

	ret = ppmp_smc(SMC_DRM_PPMP_PROT, virt_to_phys(protdesc), 0, 0);
	if (ret) {
		dma_unmap_single(dev, phys_to_dma(dev, virt_to_phys(protdesc)),
				 sizeof(*protdesc), DMA_TO_DEVICE);
		hpa_iova_free(dma_addr, size);
		pr_err("CMD %#x (err=%#lx,dva=%#x,size=%#lx,cnt=%u,flg=%u,phy=%#lx)\n",
		       SMC_DRM_PPMP_UNPROT, ret, protdesc->dma_addr,
		       protdesc->chunk_size, protdesc->chunk_count,
		       protdesc->flags, protdesc->bus_address);
		return -EACCES;
	}

	return 0;
}

static int hpa_secure_unprotect(struct buffer_prot_info *protdesc, struct device *dev)
{
	unsigned long size = protdesc->chunk_count * protdesc->chunk_size;
	unsigned long ret;

	dma_unmap_single(dev, phys_to_dma(dev, virt_to_phys(protdesc)),
			 sizeof(*protdesc), DMA_TO_DEVICE);

	ret = ppmp_smc(SMC_DRM_PPMP_UNPROT, virt_to_phys(protdesc), 0, 0);
	if (ret) {
		pr_err("CMD %#x (err=%#lx,dva=%#x,size=%#lx,cnt=%u,flg=%u,phy=%#lx)\n",
		       SMC_DRM_PPMP_UNPROT, ret, protdesc->dma_addr,
		       protdesc->chunk_size, protdesc->chunk_count,
		       protdesc->flags, protdesc->bus_address);
		return -EACCES;
	}
	/*
	 * retain the secure device address if unprotection to its area fails.
	 * It might be unusable forever since we do not know the state of the
	 * secure world before returning error from ppmp_smc() above.
	 */
	hpa_iova_free(protdesc->dma_addr, size);

	return 0;
}

static void *hpa_heap_protect(struct hpa_dma_heap *hpa_heap, struct page **pages,
			      unsigned int nr_pages)
{
	struct buffer_prot_info *protdesc;
	struct device *dev = dma_heap_get_dev(hpa_heap->dma_heap);
	unsigned long *paddr_array;
	unsigned int array_size = 0;
	int i, ret;

	protdesc = kmalloc(sizeof(*protdesc), GFP_KERNEL);
	if (!protdesc)
		return ERR_PTR(-ENOMEM);

	if (nr_pages == 1) {
		protdesc->bus_address = page_to_phys(pages[0]);
	} else {
		paddr_array = kmalloc_array(nr_pages, sizeof(*paddr_array), GFP_KERNEL);
		if (!paddr_array) {
			kfree(protdesc);
			return ERR_PTR(-ENOMEM);
		}

		for (i = 0; i < nr_pages; i++)
			paddr_array[i] = page_to_phys(pages[i]);

		protdesc->bus_address = virt_to_phys(paddr_array);

		kmemleak_ignore(paddr_array);
		array_size = nr_pages * sizeof(*paddr_array);
		dma_map_single(dev, paddr_array, array_size, DMA_TO_DEVICE);
	}

	protdesc->chunk_count = nr_pages,
	protdesc->flags = hpa_heap->protection_id;
	protdesc->chunk_size = HPA_CHUNK_SIZE;

	ret = hpa_secure_protect(protdesc, dev);
	if (ret) {
		if (protdesc->chunk_count > 1) {
			dma_unmap_single(dev, phys_to_dma(dev, protdesc->bus_address),
					 array_size, DMA_TO_DEVICE);
			kfree(paddr_array);
		}
		kfree(protdesc);
		return ERR_PTR(ret);
	}

	return protdesc;
}

static int hpa_heap_unprotect(void *priv, struct device *dev)
{
	struct buffer_prot_info *protdesc = priv;
	int ret = 0;

	if (!priv)
		return 0;

	ret = hpa_secure_unprotect(protdesc, dev);

	if (protdesc->chunk_count > 1) {
		dma_unmap_single(dev, phys_to_dma(dev, protdesc->bus_address),
				 sizeof(unsigned long) * protdesc->chunk_count, DMA_TO_DEVICE);
		kfree(phys_to_virt(protdesc->bus_address));
	}
	kfree(protdesc);

	return ret;
}

static int hpa_compare_pages(const void *p1, const void *p2)
{
	if (*((unsigned long *)p1) > (*((unsigned long *)p2)))
		return 1;
	else if (*((unsigned long *)p1) < (*((unsigned long *)p2)))
		return -1;
	return 0;
}

static struct hpa_dma_buffer* hpa_dma_buffer_alloc(struct hpa_dma_heap *hpa_heap,
						   size_t size, unsigned long nents)
{
	struct hpa_dma_buffer *buffer;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	if (sg_alloc_table(&buffer->sg_table, nents, GFP_KERNEL)) {
		pr_err("failed to allocate sgtable with %u entry\n", nents);
		kfree(buffer);
		return ERR_PTR(-ENOMEM);
	}

	buffer->heap = hpa_heap;

	return buffer;
}

void hpa_dma_buffer_free(struct hpa_dma_buffer *buffer)
{
	sg_free_table(&buffer->sg_table);
	kfree(buffer);
}

static struct sg_table *hpa_dup_sg_table(struct sg_table *table)
{
	struct sg_table *new_table;
	int ret, i;
	struct scatterlist *sg, *new_sg;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(new_table, table->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sgtable_sg(table, sg, i) {
		sg_set_page(new_sg, sg_page(sg), sg->length, sg->offset);
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static struct sg_table *hpa_heap_map_dma_buf(struct dma_buf_attachment *a,
					     enum dma_data_direction direction)
{
	struct hpa_dma_buffer *buffer = a->dmabuf->priv;
	struct sg_table *table;
	int ret = 0;

	table = hpa_dup_sg_table(&buffer->sg_table);
	if (IS_ERR(table))
		return table;

	if (dev_iommu_fwspec_get(a->dev)) {
		struct buffer_prot_info *info = buffer->priv;

		sg_dma_address(table->sgl) = info->dma_addr;
		sg_dma_len(table->sgl) = info->chunk_count * info->chunk_size;
		table->nents = 1;
	} else {
		ret = dma_map_sgtable(a->dev, table, direction, a->dma_map_attrs);
		if (ret) {
			sg_free_table(table);
			kfree(table);

			return ERR_PTR(ret);
		}
	}

	return table;
}

static void hpa_heap_unmap_dma_buf(struct dma_buf_attachment *a,
				   struct sg_table *table,
				   enum dma_data_direction direction)
{
	if (!dev_iommu_fwspec_get(a->dev))
		dma_unmap_sgtable(a->dev, table, direction, a->dma_map_attrs);

	sg_free_table(table);
	kfree(table);
}

static int hpa_heap_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	pr_err("mmap() to protected buffer is not allowed\n");
	return -EACCES;
}

static void *hpa_heap_vmap(struct dma_buf *dmabuf)
{
	pr_err("vmap() to protected buffer is not allowed\n");
	return ERR_PTR(-EACCES);
}

static void hpa_heap_dma_buf_release(struct dma_buf *dmabuf)
{
	struct hpa_dma_buffer *buffer = dmabuf->priv;
	struct device *dev = dma_heap_get_dev(buffer->heap->dma_heap);
	struct sg_table *sgt = &buffer->sg_table;
	struct scatterlist *sg;
	int i;
	int unprot_err;

	unprot_err = hpa_heap_unprotect(buffer->priv, dev);

	if (!unprot_err)
		for_each_sgtable_sg(sgt, sg, i)
			__free_pages(sg_page(sg), HPA_DEFAULT_ORDER);

	hpa_dma_buffer_free(buffer);
}

#define HPA_HEAP_FLAG_PROTECTED BIT(1)

static int hpa_heap_dma_buf_get_flags(struct dma_buf *dmabuf, unsigned long *flags)
{
	*flags = HPA_HEAP_FLAG_PROTECTED;

	return 0;
}

const struct dma_buf_ops hpa_dma_buf_ops = {
	.map_dma_buf = hpa_heap_map_dma_buf,
	.unmap_dma_buf = hpa_heap_unmap_dma_buf,
	.mmap = hpa_heap_mmap,
	.vmap = hpa_heap_vmap,
	.release = hpa_heap_dma_buf_release,
	.get_flags = hpa_heap_dma_buf_get_flags,
};

static void hpa_cache_flush(struct device *dev, struct hpa_dma_buffer *buffer)
{
	dma_map_sgtable(dev, &buffer->sg_table, DMA_TO_DEVICE, 0);
	dma_unmap_sgtable(dev, &buffer->sg_table, DMA_FROM_DEVICE, 0);
}

static void hpa_pages_clean(struct sg_table *sgt)
{
	struct sg_page_iter piter;
	struct page *p;
	void *vaddr;

	for_each_sgtable_page(sgt, &piter, 0) {
		p = sg_page_iter_page(&piter);
		vaddr = kmap_atomic(p);
		memset(vaddr, 0, PAGE_SIZE);
		kunmap_atomic(vaddr);
	}
}

static struct dma_buf *hpa_heap_allocate(struct dma_heap *heap, unsigned long len,
					 unsigned long fd_flags, unsigned long heap_flags)
{
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct hpa_dma_heap *hpa_heap = dma_heap_get_drvdata(heap);
	struct device *dev = dma_heap_get_dev(heap);
	struct hpa_dma_buffer *buffer;
	struct scatterlist *sg;
	struct dma_buf *dmabuf;
	struct page **pages;
	unsigned long size, nr_pages;
	int ret, protret = 0;
	pgoff_t pg;

	size = ALIGN(len, HPA_CHUNK_SIZE);
	nr_pages = size / HPA_CHUNK_SIZE;

	if (nr_pages > HPA_MAX_CHUNK_COUNT) {
		pr_err("Too big size %lu (HPA limited size %lu)\n", len,
		       HPA_MAX_CHUNK_COUNT * HPA_CHUNK_SIZE);
		return ERR_PTR(-EINVAL);
	}

	pages = kmalloc_array(nr_pages, sizeof(*pages), GFP_KERNEL);
	if (!pages)
		return ERR_PTR(-ENOMEM);

	ret = alloc_pages_highorder_except(HPA_DEFAULT_ORDER, pages, nr_pages,
					   hpa_exception_areas, nr_hpa_exception);
	if (ret)
		goto err_alloc;

	sort(pages, nr_pages, sizeof(*pages), hpa_compare_pages, NULL);

	buffer = hpa_dma_buffer_alloc(hpa_heap, size, nr_pages);
	if (IS_ERR(buffer)) {
		ret = PTR_ERR(buffer);
		goto err_buffer;
	}

	sg = buffer->sg_table.sgl;
	for (pg = 0; pg < nr_pages; pg++) {
		sg_set_page(sg, pages[pg], HPA_CHUNK_SIZE, 0);
		sg = sg_next(sg);
	}

	hpa_pages_clean(&buffer->sg_table);
	hpa_cache_flush(dev, buffer);

	buffer->priv = hpa_heap_protect(hpa_heap, pages, nr_pages);
	if (IS_ERR(buffer->priv)) {
		ret = PTR_ERR(buffer->priv);
		goto err_prot;
	}

	exp_info.ops = &hpa_dma_buf_ops;
	exp_info.exp_name = hpa_heap->name;
	exp_info.size = size;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;

	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto err_export;
	}
	kvfree(pages);

	return dmabuf;

err_export:
	protret = hpa_heap_unprotect(buffer->priv, dev);
err_prot:
	hpa_dma_buffer_free(buffer);
err_buffer:
	for (pg = 0; !protret && pg < nr_pages; pg++)
		__free_pages(pages[pg], HPA_DEFAULT_ORDER);
err_alloc:
	kvfree(pages);

	pr_err("failed to alloc (len %zu, %#lx %#lx) from %s heap",
	       len, fd_flags, heap_flags, hpa_heap->name);

	return ERR_PTR(ret);
}

static const struct dma_heap_ops hpa_heap_ops = {
	.allocate = hpa_heap_allocate,
};

static void hpa_add_exception_area(void)
{
	struct device_node *np;

	for_each_node_by_name(np, "dma_heap_exception_area") {
		int naddr = of_n_addr_cells(np);
		int nsize = of_n_size_cells(np);
		phys_addr_t base, size;
		const __be32 *prop;
		int len;
		int i;

		prop = of_get_property(np, "#address-cells", NULL);
		if (prop)
			naddr = be32_to_cpup(prop);

		prop = of_get_property(np, "#size-cells", NULL);
		if (prop)
			nsize = be32_to_cpup(prop);

		prop = of_get_property(np, "exception-range", &len);
		if (prop && len > 0) {
			int n_area = len / (sizeof(*prop) * (nsize + naddr));

			n_area = min_t(int, n_area, MAX_EXCEPTION_AREAS);

			for (i = 0; i < n_area ; i++) {
				base = (phys_addr_t)of_read_number(prop, naddr);
				prop += naddr;
				size = (phys_addr_t)of_read_number(prop, nsize);
				prop += nsize;

				hpa_exception_areas[i][0] = base;
				hpa_exception_areas[i][1] = base + size - 1;
			}

			nr_hpa_exception = n_area;
		}
	}
}

static int __init hpa_dma_heap_init(void)
{
	struct dma_heap_export_info exp_info;
	struct dma_heap *dma_heap;
	int ret, i = 0;

	ret = hpa_secure_iova_pool_init();
	if (ret)
		return ret;

	for (i = 0; i < ARRAY_SIZE(hpa_heaps); i++) {
		exp_info.name = hpa_heaps[i].name;
		exp_info.priv = &hpa_heaps[i];
		exp_info.ops = &hpa_heap_ops;

		dma_heap = dma_heap_add(&exp_info);
		if (IS_ERR(dma_heap))
			return PTR_ERR(dma_heap);

		hpa_heaps[i].dma_heap = dma_heap;
		dma_coerce_mask_and_coherent(dma_heap_get_dev(dma_heap), DMA_BIT_MASK(36));

		pr_info("Registered %s dma-heap successfully\n", hpa_heaps[i].name);
	}

	hpa_add_exception_area();

	return 0;
}
module_init(hpa_dma_heap_init);
MODULE_LICENSE("GPL v2");
