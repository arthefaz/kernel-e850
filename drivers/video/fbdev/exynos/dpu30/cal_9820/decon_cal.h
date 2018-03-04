/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Header file for Exynos9820 DECON CAL
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SAMSUNG_DECON_CAL_H__
#define __SAMSUNG_DECON_CAL_H__

#if defined(CONFIG_EXYNOS_LCD_ON_UBOOT) /* CAL for bootloader */
/* TODO: bootloader dependent code will be implemented here */
#else /* CAL for OS */
#include <linux/printk.h>

extern int decon_log_level;

#define decon_err(fmt, ...)							\
	do {									\
		if (decon_log_level >= 3) {					\
			pr_err(pr_fmt(fmt), ##__VA_ARGS__);			\
		}								\
	} while (0)

#define decon_warn(fmt, ...)							\
	do {									\
		if (decon_log_level >= 4) {					\
			pr_warn(pr_fmt(fmt), ##__VA_ARGS__);			\
		}								\
	} while (0)

#define decon_info(fmt, ...)							\
	do {									\
		if (decon_log_level >= 6)					\
			pr_info(pr_fmt(fmt), ##__VA_ARGS__);			\
	} while (0)

#define decon_dbg(fmt, ...)							\
	do {									\
		if (decon_log_level >= 7)					\
			pr_info(pr_fmt(fmt), ##__VA_ARGS__);			\
	} while (0)
#endif

enum decon_idma_type {
	IDMA_G0 = 0,
	IDMA_G1,
	IDMA_VG0,
	IDMA_VG1,
	IDMA_VGF0,
	IDMA_VGF1, /* VGRF in case of Exynos9810 */
	ODMA_WB,
	MAX_DECON_DMA_TYPE,
};

#define IDMA_GF0	IDMA_G0
#define IDMA_GF1	IDMA_G1
#define IDMA_VG		IDMA_VG0
#define IDMA_VGF	IDMA_VG1
#define IDMA_VGS	IDMA_VGF0
#define IDMA_VGRFS	IDMA_VGF1

u32 DPU_DMA2CH(u32 dma);
u32 DPU_CH2DMA(u32 ch);

#endif /* __SAMSUNG_DECON_CAL_H__ */
