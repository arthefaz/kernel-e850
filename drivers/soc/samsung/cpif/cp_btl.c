/*
 * Copyright (C) 2019, Samsung Electronics.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/of_reserved_mem.h>

#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/memblock.h>

#include <linux/smc.h>

#include "modem_prj.h"
#include "cp_btl.h"

/* fops */
static int btl_open(struct inode *inode, struct file *filep)
{
	struct cp_btl *btl = container_of(filep->private_data, struct cp_btl, miscdev);

	filep->private_data = (void *)btl;

	if (!btl) {
		mif_err("btl is null\n");
		return -ENODEV;
	}

	if (!btl->mem.v_base) {
		mif_err("%s: v_base is null\n", btl->name);
		return -ENOMEM;
	}

	memset(btl->mem.v_base, 0, btl->mem.size);

	return 0;
}

static int btl_release(struct inode *inode, struct file *filep)
{
	struct cp_btl *btl = NULL;

	btl = (struct cp_btl *)filep->private_data;
	if (!btl) {
		mif_err("btl is null\n");
		return -ENODEV;
	}

	if (!btl->mem.v_base) {
		mif_err("%s: v_base is null\n", btl->name);
		return -ENOMEM;
	}

	memset(btl->mem.v_base, 0, btl->mem.size);

	return 0;
}

static ssize_t btl_read(struct file *filep, char __user *buf, size_t count, loff_t *pos)
{
	struct cp_btl *btl = NULL;
	unsigned long remainder = 0;
	int len = 0;
	int ret = 0;

	btl = (struct cp_btl *)filep->private_data;
	if (!btl) {
		mif_err("btl is null\n");
		return -ENODEV;
	}

	if ((filep->f_flags & O_NONBLOCK) && !atomic_read(&btl->active))
		return -EAGAIN;

	if (*pos < 0 || *pos >= btl->mem.size) {
		mif_err("Tried to read over %d:%lld\n", btl->mem.size, *pos);
		return 0;
	}

	if (!btl->mem.v_base) {
		mif_err("%s: v_base is null\n", btl->name);
		ret = -ENOMEM;
		goto read_exit;
	}

	remainder = btl->mem.size - *pos;
	if (remainder == 0) { /* EOF */
		mif_info("%s: %lld bytes read\n", btl->name, *pos);
		ret = 0;
		goto read_exit;
	}

	len = min_t(size_t, count, SZ_1M);
	len = min_t(unsigned long, len, remainder);
	ret = copy_to_user(buf, btl->mem.v_base + *pos, len);
	if (ret) {
		mif_err("%s: copy_to_user() error:%d", btl->name, ret);
		goto read_exit;
	}

	*pos += len;

	return len;

read_exit:
	*pos = 0;

	return ret;
}

/* Command line parameter */
static bool _is_enabled[MAX_BTL_ID] = {false, false};

static int __init btl_setup_enable_0(char *str)
{
	if (!strcmp(str, "ON") || !strcmp(str, "on"))
		_is_enabled[BTL_ID_0] = true;

	mif_info("%s:%d\n", str, _is_enabled[BTL_ID_0]);

	return 0;
}
__setup("androidboot.cp_reserved_mem=", btl_setup_enable_0);

/* Create */
static const struct file_operations btl_file_ops = {
	.open = btl_open,
	.release = btl_release,
	.read = btl_read
};

int cp_btl_create(struct cp_btl *btl, struct device *dev)
{
	struct modem_data *pdata = NULL;
	int ret = 0;

	if (!dev) {
		mif_err("dev is null\n");
		return -ENODEV;
	}

	pdata = dev->platform_data;
	if (!pdata) {
		mif_err("pdata is null\n");
		return -ENODEV;
	}

	if (!btl) {
		mif_err("btl is null\n");
		return -ENOMEM;
	}
	atomic_set(&btl->active, 0);

	mif_dt_read_string(dev->of_node, "cp_btl_node_name", btl->name);

	btl->id = pdata->cp_num;
	if (btl->id >= MAX_BTL_ID) {
		mif_err("id is over max:%d\n", btl->id);
		ret = -EINVAL;
		goto create_exit;
	}
	if (!_is_enabled[btl->id]) {
		mif_err("CP BTL is disabled for %d\n", btl->id);
		ret = 0;
		goto create_exit;
	}
	btl->enabled = true;
	btl->link_type = pdata->link_type;

	mif_info("name:%s id:%d link:%d\n", btl->name, btl->id, btl->link_type);
	switch (btl->link_type) {
	case LINKDEV_SHMEM:
		btl->mem.v_base = cp_shmem_get_region(btl->id, SHMEM_BTL);
		if (!btl->mem.v_base) {
			mif_err("cp_shmem_get_region() error:v_base\n");
			ret = -ENOMEM;
			goto create_exit;
		}
		btl->mem.size = cp_shmem_get_size(btl->id, SHMEM_BTL);

		/* BAAW */
		exynos_smc(SMC_ID_CLK, SSS_CLK_ENABLE, 0, 0);
		ret = exynos_smc(SMC_ID, CP_BOOT_REQ_CP_RAM_LOGGING, 0, 0);
		if (ret) {
			mif_err("exynos_smc() error:%d\n", ret);
			goto create_exit;
		}
		exynos_smc(SMC_ID_CLK, SSS_CLK_DISABLE, 0, 0);
		break;

	case LINKDEV_PCIE:
		break;

	default:
		mif_err("link_type error:%d\n", btl->link_type);
		ret = -EINVAL;
		goto create_exit;
	}

	btl->miscdev.minor = MISC_DYNAMIC_MINOR;
	btl->miscdev.name = btl->name;
	btl->miscdev.fops = &btl_file_ops;
	ret = misc_register(&btl->miscdev);
	if (ret) {
		mif_err("misc_register() error for %s:%d", btl->name, ret);
		goto create_exit;
	}

	atomic_set(&btl->active, 1);

	return 0;

create_exit:
	if (btl->mem.v_base)
		vunmap(btl->mem.v_base);

#if !defined(CONFIG_SOC_EXYNOS9820)
	cp_shmem_release_rmem(btl->id, SHMEM_BTL);
#endif

	return ret;
}

int cp_btl_destroy(struct cp_btl *btl)
{
	if (!btl) {
		mif_err("btl is null\n");
		return -ENODEV;
	}

	if (btl->mem.v_base)
		vunmap(btl->mem.v_base);

	misc_deregister(&btl->miscdev);

	return 0;
}
