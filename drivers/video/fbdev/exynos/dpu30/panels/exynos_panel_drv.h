/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Header file for Samsung EXYNOS Panel Driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __EXYNOS_PANEL_DRV_H__
#define __EXYNOS_PANEL_DRV_H__

#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/backlight.h>
#include <linux/regulator/consumer.h>
#include <video/mipi_display.h>
#include <media/v4l2-subdev.h>
#include "exynos_panel.h"
#include "../dsim.h"
#include "../decon.h"

extern int dpu_panel_log_level;

#define DPU_DEBUG_PANEL(fmt, args...)					\
	do {								\
		if (dpu_panel_log_level >= 7)				\
			dpu_debug_printk("PANEL", fmt,  ##args);	\
	} while (0)

#define DPU_INFO_PANEL(fmt, args...)					\
	do {								\
		if (dpu_panel_log_level >= 6)				\
			dpu_debug_printk("PANEL", fmt,  ##args);	\
	} while (0)

#define DPU_ERR_PANEL(fmt, args...)					\
	do {								\
		if (dpu_panel_log_level >= 3)				\
			dpu_debug_printk("PANEL", fmt, ##args);		\
	} while (0)

#define MAX_REGULATORS		3
#define MAX_PANEL_SUPPORT	10

extern struct exynos_panel_device *panel_drvdata;
extern struct exynos_panel_ops panel_s6e3ha9_ops;
extern struct exynos_panel_ops panel_s6e3ha8_ops;
extern struct exynos_panel_ops panel_s6e3fa0_ops;

struct exynos_panel_resources {
	int lcd_reset;
	int lcd_power[2];
	struct regulator *regulator[MAX_REGULATORS];
};

struct exynos_panel_ops {
	u32 id;
	int (*suspend)(struct exynos_panel_device *panel);
	int (*displayon)(struct exynos_panel_device *panel);
	int (*mres)(struct exynos_panel_device *panel, int mres_idx);
	int (*doze)(struct exynos_panel_device *panel);
	int (*doze_suspend)(struct exynos_panel_device *panel);
	int (*dump)(struct exynos_panel_device *panel);
	int (*read_state)(struct exynos_panel_device *panel);
};

struct exynos_panel_device {
	u32 id;		/* panel device id */
	bool found;	/* found connected panel or not */
	struct device *dev;
	struct v4l2_subdev sd;
	struct mutex ops_lock;
	struct exynos_panel_resources res;
	struct backlight_device *bl;
	struct exynos_panel_info lcd_info;
	struct exynos_panel_ops *ops;
};

static inline struct exynos_panel_device *get_panel_drvdata(void)
{
	return panel_drvdata;
}

#define call_panel_ops(q, op, args...)				\
	(((q)->ops->op) ? ((q)->ops->op(args)) : 0)

#define EXYNOS_PANEL_IOC_REGISTER	_IOW('P', 0, u32)
#define EXYNOS_PANEL_IOC_POWERON	_IOW('P', 1, u32)
#define EXYNOS_PANEL_IOC_POWEROFF	_IOW('P', 2, u32)
#define EXYNOS_PANEL_IOC_RESET		_IOW('P', 3, u32)
#define EXYNOS_PANEL_IOC_DISPLAYON	_IOW('P', 4, u32)
#define EXYNOS_PANEL_IOC_SUSPEND	_IOW('P', 5, u32)
#define EXYNOS_PANEL_IOC_MRES		_IOW('P', 6, u32)
#define EXYNOS_PANEL_IOC_DOZE		_IOW('P', 7, u32)
#define EXYNOS_PANEL_IOC_DOZE_SUSPEND	_IOW('P', 8, u32)
#define EXYNOS_PANEL_IOC_DUMP		_IOW('P', 9, u32)
#define EXYNOS_PANEL_IOC_READ_STATE	_IOR('P', 10, u32)

#endif /* __EXYNOS_PANEL_DRV_H__ */
