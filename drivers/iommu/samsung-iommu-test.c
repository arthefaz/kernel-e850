// SPDX-License-Identifier: GPL-2.0

#include <linux/debugfs.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <asm/io.h>

/* Base address of DPU IOMMU (non-secure block) */
#define SYSMMU_BASE		0x130c0000
#define SYSMMU_SIZE		0x10000

/* Offsets for registers for initiating the "traffic emulation" */
#define MMU_STATUS		0x0008
#define MMU_EMU_PRELOAD		0x3040
#define MMU_EMU_SHOT		0x3044

/* Offsets for registers to check the translation result */
#define MMU_TLB_READ		0x3000
#define MMU_TLB_VPN		0x3004
#define MMU_TLB_PPN		0x3008
#define MMU_TLB_ATTR		0x300C

/* No ongoing translation or invalidation and AXI interface is unblocked */
#define STATUS_READY		0x0
/* RW bit; 1 - write request, 0 - read request */
#define PRELOAD_WRITE		(1 << 20)
#define PRELOAD_READ		(0 << 20)
/* Virtual page number for emulating address translation (VA[31:12]) */
#define PRELOAD_VPN_OFFSET	0
/* AXI port ID (AXI port information to be emulated) */
#define SHOT_PID_OFFSET		16
/* AXI ID (target stream ID for emulating address translation) */
#define SHOT_SID_OFFSET		0

#define MAPPING_SIZE		SZ_16M
/* Test value */
#define MAGIC_NUMBER		0x11223344

struct iommu_test {
	struct platform_device *pdev;
	struct device *dev;
	struct dentry *dir;			/* DebugFS dir */

	/* Objects needed for allocation and mapping the memory */
	struct dma_heap *dma_heap;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *dma_att;
	struct sg_table *sg_table;

	/* Addresses used for testing (start of mapped memory) */
	dma_addr_t iova;			/* IOMMU VA address */
	phys_addr_t pa;				/* Physica Address in RAM */
	u32 *va;				/* Virtual Address in kernel */

	void __iomem *reg_base;			/* IOMMU registers (mapped) */
};

/* ---- private ------------------------------------------------------------- */

static void iommu_test_do_translation(const struct iommu_test *obj)
{
	const u32 rw	= PRELOAD_READ;
	const u32 vpn	= obj->iova >> 12; /* VPN = VA[31:12] */
	const u32 pid	= 0x0;
	const u32 sid	= 0x2;

	pr_info("initiate IOMMU translation (\"traffic emulation\")\n");

	/* Wait until emulation is available */
	while (ioread32(obj->reg_base + MMU_STATUS) != STATUS_READY)
		;

	/* Preload for emulation */
	iowrite32(rw | vpn, obj->reg_base + MMU_EMU_PRELOAD);

	/* Issue emulation (initiate the translation) */
	iowrite32(sid | (pid << SHOT_PID_OFFSET), obj->reg_base + MMU_EMU_SHOT);
}

static void iommu_test_check_result(const struct iommu_test *obj)
{
	const u32 pg_off = obj->iova & 0xfff; /* "page offset" part of addr */
	const size_t way = 0; /* only way #0 is available for all TLBs */
	size_t tlb, set, entry;
	u32 read_val;
	u32 vpn, ppn, attr;
	u32 iova;
	u64 pa;
	u32 *va;

	/*
	 * Dump TLBs.
	 *   - We used SID = 0x2, which matches TLB #1
	 *   - TLB #1: 1 way, 2 sets, 4 lines
	 *   - Check TLB #0 (default one) and TLB #1
	 */
	pr_info("Dump TLBs:\n");
	pr_info("  [TLB][SET][ENTRY]: VPN, PPN, ATTR\n");
	for (tlb = 0; tlb < 2; ++tlb) {
	for (set = 0; set < 2; ++set) {
	for (entry = 0; entry < 4; ++entry) {
		read_val = (tlb << 20) | (way << 8) | (entry << 16) |
			   (set << 0);
		while (ioread32(obj->reg_base + MMU_STATUS) != STATUS_READY)
			;

		iowrite32(read_val, obj->reg_base + MMU_TLB_READ);
		vpn = ioread32(obj->reg_base + MMU_TLB_VPN);
		ppn = ioread32(obj->reg_base + MMU_TLB_PPN);
		attr = ioread32(obj->reg_base + MMU_TLB_ATTR);
		pr_info("  [%zu][%zu][%zu]: 0x%x, 0x%x, 0x%x\n",
			tlb, set, entry, vpn, ppn, attr);
	}
	}
	}

	/* Check IOVA and PA for correctness */
	tlb = 0;
	set = 0;
	entry = 0;
	read_val = (tlb << 20) | (way << 8) | (entry << 16) | (set << 0);
	while (ioread32(obj->reg_base + MMU_STATUS) != STATUS_READY)
		;
	iowrite32(read_val, obj->reg_base + MMU_TLB_READ);
	vpn = ioread32(obj->reg_base + MMU_TLB_VPN) & 0x0fffff;
	ppn = ioread32(obj->reg_base + MMU_TLB_PPN) & 0xffffff;
	iova = (vpn << 12) + pg_off;
	pa = ((u64)ppn << 12) + pg_off;
	va = phys_to_virt(pa);
	pr_info("Actual addresses:\n");
	pr_info("Translated: IOVA = 0x%x, PA = 0x%llx\n", iova, pa);
	pr_info("Mapped:     IOVA = %pad, PA = %pa\n", &obj->iova, &obj->pa);
	pr_info("Verifying the magic by reading: %s\n",
		*va == MAGIC_NUMBER ? "OK" : "FAIL");
}

static int iommu_test_create_mapping(struct iommu_test *obj)
{
	const size_t size = PAGE_ALIGN(MAPPING_SIZE);
	int ret;

	obj->dma_heap = dma_heap_find("system");
	if (!obj->dma_heap) {
		dev_warn(obj->dev, "can't find system heap, deferring probe\n");
		return -EPROBE_DEFER;
	}

	obj->dma_buf = dma_heap_buffer_alloc(obj->dma_heap, size, O_RDWR, 0);
	if (IS_ERR(obj->dma_buf)) {
		ret = -ENOMEM;
		dev_err(obj->dev, "dma_heap_buffer_alloc() failed\n");
		goto err_buf_alloc;
	}

	obj->dma_att = dma_buf_attach(obj->dma_buf, obj->dev);
	if (IS_ERR_OR_NULL(obj->dma_att)) {
		ret = PTR_ERR(obj->dma_att);
		dev_err(obj->dev, "dma_buf_attach() failed: %d\n", ret);
		goto err_buf_attach;
	}

	/* This call leads to samsung_sysmmu_map() */
	obj->sg_table = dma_buf_map_attachment(obj->dma_att, DMA_TO_DEVICE);
	if (IS_ERR_OR_NULL(obj->sg_table)) {
		ret = PTR_ERR(obj->sg_table);
		dev_err(obj->dev, "dma_buf_map_attachment() failed: %d\n", ret);
		goto err_buf_map;
	}

	/* Get IOVA start address of mapped memory, will be used by IOMMU */
	obj->iova = sg_dma_address(obj->sg_table->sgl);
	if (IS_ERR_VALUE(obj->iova)) {
		ret = -EINVAL;
		dev_err(obj->dev, "sg_dma_address() failed: %pa\n", &obj->iova);
		goto err_buf_map;
	}

	/* Get PA start address of mapped memory */
	obj->pa = sg_phys(obj->sg_table->sgl);
	/* RAM addresses are already mapped in kernel pgtable */
	obj->va = phys_to_virt(obj->pa);
	/* Print mapped mem start addr */
	dev_info(obj->dev, "IOVA = %pad, PA = %pa\n", &obj->iova, &obj->pa);

	return 0;

err_buf_map:
	dma_buf_detach(obj->dma_buf, obj->dma_att);
err_buf_attach:
	dma_buf_put(obj->dma_buf);
err_buf_alloc:
	dma_heap_put(obj->dma_heap);
	return ret;
}

static void iommu_test_free_mapping(struct iommu_test *obj)
{
	dma_buf_unmap_attachment(obj->dma_att, obj->sg_table, DMA_TO_DEVICE);
	dma_buf_detach(obj->dma_buf, obj->dma_att);
	dma_buf_put(obj->dma_buf);
	dma_heap_put(obj->dma_heap);
}

/* ---- DebugFS ------------------------------------------------------------- */

static inline struct iommu_test *to_iommu_test(struct file *file)
{
	/*
	 * file->private_data is set in simple_open(), in turn using data set in
	 * debugfs_create_file().
	 */
	return (struct iommu_test *)file->private_data;
}

static ssize_t iommu_test_read(struct file *file, char __user *buf,
			       size_t count, loff_t *pos)
{
	struct iommu_test *obj = to_iommu_test(file);

	*obj->va = MAGIC_NUMBER; /* write magic number to tested RAM addr */
	iommu_test_do_translation(obj);
	iommu_test_check_result(obj);

	return 0;
}

static const struct file_operations iommu_test_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = iommu_test_read,
	.write = NULL,
};

/* ---- Module -------------------------------------------------------------- */

static int iommu_test_probe(struct platform_device *pdev)
{
	struct iommu_test *drv;
	struct dentry *file;
	int ret;

	/*
	 * Don't try to init this module before system heap is ready (fail
	 * fast). That prevents unnecessary SysMMU init/deinit sequence and
	 * simplifies tracing of SysMMU driver IO operations if needed.
	 */
	if (!dma_heap_find("system")) {
		dev_warn(&pdev->dev, "can't find system heap, deferring\n");
		return -EPROBE_DEFER;
	}

	drv = kzalloc(sizeof(*drv), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(drv))
		return -ENOMEM;

	platform_set_drvdata(pdev, drv);
	drv->pdev = pdev;
	drv->dev = &pdev->dev;

	pm_runtime_enable(drv->dev);
	pm_runtime_get_sync(drv->dev);

	ret = iommu_test_create_mapping(drv);
	if (ret)
		goto err_create_mapping;

	drv->dir = debugfs_create_dir("iommu_test", NULL);
	if (IS_ERR_OR_NULL(drv->dir)) {
		ret = PTR_ERR(drv->dir);
		dev_err(drv->dev, "Error creating DebugFS dir: %d\n", ret);
		goto err_create_dir;
	}

	file = debugfs_create_file("test", S_IRUGO, drv->dir, drv,
				   &iommu_test_fops);
	if (IS_ERR_OR_NULL(file)) {
		ret = PTR_ERR(file);
		dev_err(drv->dev, "Error creating DebugFS file: %d\n", ret);
		goto err_create_file;
	}

	drv->reg_base = ioremap(SYSMMU_BASE, SYSMMU_SIZE);
	if (!drv->reg_base) {
		ret = -ENOMEM;
		goto err_create_file;
	}

	return 0;

err_create_file:
	debugfs_remove_recursive(drv->dir);
err_create_dir:
	iommu_test_free_mapping(drv);
err_create_mapping:
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
	kfree(drv);
	return ret;
}

static int iommu_test_remove(struct platform_device *pdev)
{
	struct iommu_test *drv = platform_get_drvdata(pdev);

	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
	debugfs_remove_recursive(drv->dir);
	iounmap(drv->reg_base);
	iommu_test_free_mapping(drv);
	kfree(drv);

	return 0;
}

static const struct of_device_id iommu_test_of_match[] = {
	{ .compatible = "samsung,iommu-test" },
	{},
};
MODULE_DEVICE_TABLE(of, iommu_test_of_match);

static struct platform_driver iommu_test_driver = {
	.probe = iommu_test_probe,
	.remove = iommu_test_remove,
	.driver = {
		.name = "samsung-iommu-test",
		.of_match_table = of_match_ptr(iommu_test_of_match),
	},
};

module_platform_driver(iommu_test_driver);

MODULE_AUTHOR("Sam Protsenko <semen.protsenko@linaro.org>");
MODULE_DESCRIPTION("Samsung IOMMU test driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:samsung-iommu-test");
MODULE_IMPORT_NS(DMA_BUF);
