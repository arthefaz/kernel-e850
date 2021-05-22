// SPDX-License-Identifier: GPL-2.0
/*
 *
 *  Copyright (c) 2020 Linaro Limited
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 */
#ifndef _LT8912B_H
#define _LT8912B_H

#define LT8912_CHIP_ID			0x12B2

#define LT8912B_I2C_MAIN_ADDR		0x48
#define LT8912B_I2C_MAIN_IDX		0

#define LT8912B_I2C_CEC_DSI_ADDR	0x49
#define LT8912B_I2C_CEC_DSI_IDX		1

#define LT8912B_I2C_TX_ADDR		0x4a
#define LT8912B_I2C_TX_IDX		2

#define LT8912B_MAX_I2C_CLIENTS		3
struct video_timing {
	unsigned short hfp;
	unsigned short hs;
	unsigned short hbp;
	unsigned short hact;
	unsigned short htotal;
	unsigned short vfp;
	unsigned short vs;
	unsigned short vbp;
	unsigned short vact;
	unsigned short vtotal;
	unsigned int pclk_khz;
};


struct lt8912b {
	struct device *dev;
	struct mutex i2c_lock;

	struct gpio_desc *reset_gpio;
	struct gpio_desc *mipi_switch_gpio;
	struct gpio_desc *lt8912_irq_gpio;
	struct gpio_desc *hpd_irq_gpio;
	int lt8912_irq;
	int hpd_irq;

	struct i2c_client *i2c_client[LT8912B_MAX_I2C_CLIENTS];
	struct regmap *regmap[LT8912B_MAX_I2C_CLIENTS];

	struct video_timing *vid_mode;
	unsigned int hsync_l, hsync_h, vsync_l, vsync_h;
};

#endif
