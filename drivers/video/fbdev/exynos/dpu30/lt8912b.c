// SPDX-License-Identifier: GPL-2.0
/*
 *
 *  Copyright (c) 2020 Linaro Limited
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/regmap.h>
#include <linux/pm.h>
#include "lt8912b.h"

//hfp, hs, hbp, hact, htotal, vfp, vs, vbp, vact, vtotal, pixclk
static struct video_timing video_800x480_60Hz	= {40,48,40,800,928,13,1,31,480,525,30000};
static struct video_timing video_1280x720_60Hz	= {110,40, 220,1280,  1650,  5,  5,  20, 720,   750};
static struct video_timing video_1920x1080_60Hz	= { 88, 44, 148, 1920, 2200, 4, 5, 36, 1080, 1125, 148500 };
static struct video_timing video_1366x768_60Hz	= {14, 56,  64,1366,  1500,  1,  3,  28, 768,   800};
static struct video_timing video_1280x800_60Hz	= {48,32,80,1280,1440,3,6,14,800,823,71100};
static struct video_timing video_1920x1080_30Hz	= { 88, 44, 148, 1920, 2200, 5, 5, 36, 1080, 1125, 148500 };

/* handling only the above resolutions for now
static struct video_timing video_640x480_60Hz	= { 8, 96,  40, 640,   800, 33,  2,  10, 480,   525};
static struct video_timing video_720x480_60Hz	= {16, 62,  60, 720,   858,  9,  6,  30, 480,   525};
static struct video_timing video_1920x1080_60Hz	= {88, 44, 148,1920,  2200,  4,  5,  36, 1080, 1125};
static struct video_timing video_3840x1080_60Hz	= {176,88, 296,3840,  4400,  4,  5,  36, 1080, 1125};
static struct video_timing video_3840x2160_30Hz	= {176,88, 296,3840,  4400,  8,  10, 72, 2160, 2250};
*/

static struct lt8912b *lt;

static void lt8912b_reset(struct lt8912b *lt)
{
	pr_err("### %s: resetting lt8912b\n", __func__);
	gpiod_direction_output(lt->reset_gpio, 1);
	msleep(120);
	gpiod_direction_output(lt->reset_gpio, 0);
	msleep(120);
}

static bool lt8912b_get_hpd(struct lt8912b *lt)
{
	unsigned int hpd_enabled;
	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0xc1, &hpd_enabled);

	pr_err("### %s: lt8912b hpd is : %s", __func__,((hpd_enabled & 0x80)==0x80)?"HIGH":"LOW");
	return((hpd_enabled & 0x80)==0x80);
}

static void lt8912b_check_dds(struct lt8912b *lt)
{
	unsigned int reg_920c, reg_920d, reg_920e, reg_920f;
	u8 i;

	for(i = 0; i < 40; i++)
	{
		regmap_read(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x0c, &reg_920c);
		regmap_read(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x0d, &reg_920d);
		regmap_read(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x0e, &reg_920e);
		regmap_read(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x0f, &reg_920f);

		pr_err("\r\n0x0c~0f = %0x, %0x, %0x, %0x\n",reg_920c, reg_920d, reg_920e, reg_920f);
		if((reg_920e == 0xd3)&&(reg_920d < 0xff)&&(reg_920d > 0xd0)) //shall update threshold here base on actual dds result.
		{
			pr_err("\r\nlvds_check_dds: stable!\n");
			break;
		}

		msleep(50);
	}
}

static irqreturn_t lt8912b_isr(int irq, void *devid)
{
	struct lt8912b *obj = devid;

	/* TODO: Implement LT8912 interrupt handling here */
	pr_err("### Hello from %s!\n", __func__);
	lt8912b_get_hpd(obj);

	return IRQ_HANDLED;
}

static irqreturn_t lt8912b_hpd_isr(int irq, void *devid)
{
	struct lt8912b *obj = devid;
	unsigned int hsync_l, hsync_h, vsync_l, vsync_h;
	bool hpd = false;

	/* TODO: Implement HPD interrupt handling here */
	pr_err("### Hello from %s!\n", __func__);
	hpd = lt8912b_get_hpd(obj);

	if (hpd) {
		regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x9c, &hsync_l);
		regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x9d, &hsync_h);
		regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x9e, &vsync_l);
		regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x9f, &vsync_h);

		dev_err(lt->dev, "MIPI Input detect values: hsync: %0x:%0x vsync %0x:%0x\n", hsync_l, hsync_h, vsync_l, vsync_h);
	}

	return IRQ_HANDLED;
}

#define LT8912B_DEBUG 1
static void reg_single_read_and_print(char *gp_name, struct regmap *regmap, unsigned int reg_read)
{
#if LT8912B_DEBUG
	unsigned int value;

	regmap_read(regmap, reg_read, &value);
	dev_err(lt->dev, "%s reg %0x: value: %0x\n", gp_name, reg_read, value);
#else
	dev_err(lt->dev, "%s reg %0x: debug not set\n", gp_name, reg_read);
#endif
	return;
}


static void reg_read_and_print(char *gp_name, struct regmap *regmap, const struct reg_sequence *seq, int count)
{
#if LT8912B_DEBUG
	unsigned int value;
	int i;

	for (i = 0; i < count; i++) {
		regmap_read(regmap, seq[i].reg, &value);
		dev_err(lt->dev, "%s reg %0x: value: %0x\n", gp_name, seq[i].reg, value);
	}
#else
	dev_err(lt->dev, "%s regs not read: debug not set\n", gp_name);
#endif
	return;
}

// called with mutex held
static int lt8912b_lvds_config_init(struct lt8912b *lt)
{
	/* program lvds in bypass mode, and keep on */
	const struct reg_sequence seq[] = {
		{0x44, 0x30},
		{0x51, 0x05},
		{0x50, 0x24},
		{0x51, 0x2d},
		{0x52, 0x04},
		{0x69, 0x0e},
		{0x69, 0x8e},
		{0x6a, 0x00},
		{0x6c, 0xb8},
		{0x6b, 0x51},
		{0x04, 0xfb},
		{0x04, 0xff},
		{0x7f, 0x00},
		{0xa8, 0x13},
		{0x02, 0xf7},
		{0x02, 0xff},
		{0x03, 0xcb},
		{0x03, 0xfb},
		{0x03, 0xff},
	};

	int ret = 0;

	ret = regmap_multi_reg_write(lt->regmap[LT8912B_I2C_MAIN_IDX], seq, ARRAY_SIZE(seq));

	reg_read_and_print("LVDS Config", lt->regmap[LT8912B_I2C_MAIN_IDX], seq, ARRAY_SIZE(seq));

	return ret;
};

static int lt8912b_avi_info_frame(struct lt8912b *lt)
{
	const struct reg_sequence seq[] = {
		{0x3c, 0x41}, 	//enable null package
		{0x43, 0x27}, 	//PB0:check sum
		{0x44, 0x10},	//PB1
		{0x45, 0x28},	//PB2
		{0x46, 0x00},	//PB3
		{0x47, 0x10},	//PB4
	};

	int ret = 0;

	ret = regmap_multi_reg_write(lt->regmap[LT8912B_I2C_TX_IDX], seq, ARRAY_SIZE(seq));

	reg_read_and_print("AVI Info", lt->regmap[LT8912B_I2C_TX_IDX], seq, ARRAY_SIZE(seq));

	ret = regmap_write(lt->regmap[LT8912B_I2C_MAIN_IDX], 0xab, 0x03); // sync polarity +

	reg_single_read_and_print("sync polarity: ", lt->regmap[LT8912B_I2C_MAIN_IDX], 0xab);
	return ret;
};

static int lt8912b_init_config(struct lt8912b *lt)
{
	int ret = 0;
	const struct reg_sequence seq[] = {
		/* Digital clock en*/
		{0x02, 0xf7},
		{0x08, 0xff},
		{0x09, 0xff},
		{0x0a, 0xff},
		{0x0b, 0x7c},
		{0x0c, 0xff},

		/*Tx Analog*/
		{0x31, 0xa1},
		{0x32, 0xa1},
		{0x33, 0x0c},
		{0x37, 0x00},
		{0x38, 0x22},
		{0x60, 0x82},

		/*Cbus Analog*/
		{0x39, 0x45},
		{0x3a, 0x00},
		{0x3b, 0x00},

		/*HDMI Pll Analog*/
		{0x44, 0x31},
		{0x55, 0x44},
		{0x57, 0x01},
		{0x5a, 0x02},

		/*MIPI Analog*/
		{0x3e, 0xd6},
		{0x3f, 0xd4},
		{0x41, 0x3c},
//		{0xB2, 0x00},
	};

	ret = regmap_multi_reg_write(lt->regmap[LT8912B_I2C_MAIN_IDX], seq, ARRAY_SIZE(seq));

	/* Read back and print DigitalClockEn */
	reg_read_and_print("DigitalClockEn etc", lt->regmap[LT8912B_I2C_MAIN_IDX], seq, ARRAY_SIZE(seq));

	return ret;
}

static int lt8912b_mipi_basic_config(struct lt8912b *lt)
{
	u8 lanes = 4;
	int ret = 0;
	const struct reg_sequence seq[] = {
		{0x10, 0x01},
		{0x11, 0x08},
		{0x14, 0x00},
		{0x15, 0x00},
		{0x1a, 0x03},
		{0x1b, 0x03},
	};

	ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x13, lanes % 4);

	if (!ret)
		ret = regmap_multi_reg_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], seq, ARRAY_SIZE(seq));

	/* Read back and print MipiBasicSet */
	reg_read_and_print("MipiBasicSet", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], seq, ARRAY_SIZE(seq));

	return ret;
};

static int lt8912b_dds_config(struct lt8912b *lt)
{
	int ret = 0;
	const struct reg_sequence seq[] = {
		{0x4e, 0xff},
		{0x4f, 0x56},
		{0x50, 0x69},
		{0x51, 0x80},
		{0x1e, 0x4f},
		{0x1f, 0x5e},
		{0x20, 0x01},
		{0x21, 0x2c},
		{0x22, 0x01},
		{0x23, 0xfa},
		{0x24, 0x00},
		{0x25, 0xc8},
		{0x26, 0x00},
		{0x27, 0x5e},
		{0x28, 0x01},
		{0x29, 0x2c},
		{0x2a, 0x01},
		{0x2b, 0xfa},
		{0x2c, 0x00},
		{0x2d, 0xc8},
		{0x2e, 0x00},
		{0x42, 0x64},
		{0x43, 0x00},
		{0x44, 0x04},
		{0x45, 0x00},
		{0x46, 0x59},
		{0x47, 0x00},
		{0x48, 0xf2},
		{0x49, 0x06},
		{0x4a, 0x00},
		{0x4b, 0x72},
		{0x4c, 0x45},
		{0x4d, 0x00},
		{0x52, 0x08},
		{0x53, 0x00},
		{0x54, 0xb2},
		{0x55, 0x00},
		{0x56, 0xe4},
		{0x57, 0x0d},
		{0x58, 0x00},
		{0x59, 0xe4},
		{0x5a, 0x8a},
		{0x5b, 0x00},
		{0x5c, 0x34},
		{0x1e, 0x4f},
		{0x51, 0x00},
	};

	ret = regmap_multi_reg_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], seq, ARRAY_SIZE(seq));

	/* Read back and print DDSConfig */
	reg_read_and_print("DDSConfig", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], seq, ARRAY_SIZE(seq));

	return ret;
}

static int lt8912b_rxlogic_dds_reset(struct lt8912b *lt)
{
	int ret;

	// MipiRxLogic Reset
	ret = regmap_write(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x03, 0x7f);
	usleep_range(10000, 20000);
	ret |= regmap_write(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x03, 0xff);

	// DDS reset
	ret |= regmap_write(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x05, 0xfb);
	usleep_range(10000, 20000);
	ret |= regmap_write(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x05, 0xff);

	reg_single_read_and_print("MipiRxLogicRes", lt->regmap[LT8912B_I2C_MAIN_IDX], 0x03);
	reg_single_read_and_print("DDS Reset", lt->regmap[LT8912B_I2C_MAIN_IDX], 0x05);

	return ret;
};

static int lt8912b_set_video_mode(struct lt8912b *lt, struct video_timing *vid_mode)
{
	u32 hactive, hfp, hsync, hbp, vfp, vsync, vbp, htotal, vtotal, vactive;
	u8 settle = 0x08;
	int ret = 0;

	lt->vid_mode = vid_mode;

	hactive = lt->vid_mode->hact;
	vactive = lt->vid_mode->vact;
	hfp = lt->vid_mode->hfp;
	hsync = lt->vid_mode->hs;
	hbp = lt->vid_mode->hbp;
	vfp = lt->vid_mode->vfp;
	vsync = lt->vid_mode->vs;
	vbp = lt->vid_mode->vbp;
	htotal = lt->vid_mode->htotal;
	vtotal = lt->vid_mode->vtotal;

	if (vactive <= 600)
		settle = 0x04;
	else if (vactive == 1080)
		settle = 0x0a;

	pr_err("%s: hfp: %d, hs: %d, hbp: %d, hact: %d, htotal: %d\n", __func__, hfp, hsync, hbp, hactive, htotal);
	pr_err("vfp: %d, vs: %d, vbp: %d, vtotal: %d, vact: %d, settle: %0x\n", vfp, vsync, vbp, vtotal, vactive, settle);

	ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x10, 0x01);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x11, settle);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x18, hsync);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x19, vsync);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x1c, hactive % 0x100);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x1d, hactive >> 8);

	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x2f, 0x0c);

	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x34, htotal % 0x100);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x35, htotal >> 8);

	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x36, vtotal % 0x100);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x37, vtotal >> 8);

	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x38, vbp % 0x100);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x39, vbp >> 8);

	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3a, vfp % 0x100);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3b, vfp >> 8);

	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3c, hbp % 0x100);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3d, hbp >> 8);

	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3e, hfp % 0x100);
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3f, hfp >> 8);


	/* Read back and print Video Mode */
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x18);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x19);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x1c);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x1d);

	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x2f);

	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x34);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x35);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x36);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x37);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x38);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x39);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3a);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3b);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3c);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3d);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3e);
	reg_single_read_and_print("Video Mode", lt->regmap[LT8912B_I2C_CEC_DSI_IDX], 0x3f);

	return ret;
}

static int lt8912b_parse_video_mode(struct lt8912b *lt)
{
	int ret = 0;
	unsigned int hsync_l, hsync_h, vsync_l, vsync_h;

	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x9c, &hsync_l);
	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x9d, &hsync_h);
	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x9e, &vsync_l);
	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x9f, &vsync_h);

	dev_err(lt->dev, "MIPI Input detect values: hsync: %0x:%0x vsync %0x:%0x\n", hsync_l, hsync_h, vsync_l, vsync_h);

	if((hsync_h!=lt->hsync_h)||(vsync_h!=lt->vsync_h)) /* hiht byte changed */
	{
		if(vsync_h == 0x02 && vsync_l == 0x0d)	//0x20D
		{
			ret = lt8912b_set_video_mode(lt, &video_800x480_60Hz);
			dev_err(lt->dev, "\r\nvideoformat = VESA_640x480_60");
		}
		else if(vsync_h==0x02 && vsync_l <= 0xef && vsync_l >= 0xec)//0x2EE
		{
			ret = lt8912b_set_video_mode(lt, &video_1280x720_60Hz);
			dev_err(lt->dev, "\r\nvideoformat = VESA_1280x720_60");
		}
		else if(vsync_h == 0x04 && vsync_l <= 0x67 && vsync_l >= 0x63)//0x465
		{
			ret = lt8912b_set_video_mode(lt, &video_1920x1080_30Hz); // &video_1920x1080_60Hz);
			dev_err(lt->dev, "\r\nvideoformat = VESA_1920x1080_30");
		}
		else if(vsync_h == 0x03 && vsync_l <= 0x23 && vsync_l >= 0x1d)//0x320
		{
			ret = lt8912b_set_video_mode(lt, &video_1366x768_60Hz);
			dev_err(lt->dev, "\r\nvideoformat = VESA_1366x768_60");
		}
		else if(vsync_h == 0x03 && vsync_l <= 0x3a && vsync_l >= 0x34)//0x337
		{
			ret = lt8912b_set_video_mode(lt, &video_1280x800_60Hz);
			dev_err(lt->dev, "\r\nvideoformat = VESA_1280x800_60");
		}
		else
		{
			dev_err(lt->dev, "\r\nvideoformat = video_none");
		}

		lt->hsync_l = hsync_l;
		lt->hsync_h = hsync_h;
		lt->vsync_l = vsync_l;
		lt->vsync_h = vsync_h;

	} else { /* re-program registers for current video mode */
		ret = lt8912b_set_video_mode(lt, lt->vid_mode);
	}

	if (!ret)
		ret = lt8912b_rxlogic_dds_reset(lt);

	return ret;
}

static int lt8912b_set_audio(struct lt8912b *lt)
{
	int ret = 0;
	const struct reg_sequence seq[] = {
		{0x06, 0x00},
		{0x07, 0x00},
		{0x34, 0xd2},
	};

	ret = regmap_write(lt->regmap[LT8912B_I2C_MAIN_IDX], 0xb2, 0x01); // force to HDMI
	if (!ret) {
		ret = regmap_multi_reg_write(lt->regmap[LT8912B_I2C_TX_IDX], seq, ARRAY_SIZE(seq));
	} else {
		dev_err(lt->dev, "audio force to HDMI failed\n");
		goto audio_err;
	}
	/* Read and print AudioEn settings */
	reg_single_read_and_print("AudioEn", lt->regmap[LT8912B_I2C_MAIN_IDX], 0xb2);
	reg_read_and_print("AudioEn", lt->regmap[LT8912B_I2C_TX_IDX], seq, ARRAY_SIZE(seq));

	// not handling AVI info frame for now; check for later
audio_err:
	return ret;
}

void lt8912b_init(void)
{
	int ret = 0;
	bool hpd = false;

	unsigned int version[2];

	if (!lt || !lt->vid_mode) {
		pr_err("### %s: Error: lt is not initialized\n", __func__);
		return; /* -EINVAL */
	}

	mutex_lock(&lt->i2c_lock);
	/* read chip ID */
	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x00, &version[0]);
	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x01, &version[1]);

	dev_err(lt->dev, "LT8912 ID: %02x, %02x\n",
		 version[0], version[1]);

	ret = lt8912b_init_config(lt);
	if (!ret)
		dev_err(lt->dev, "lt8912b_init_config Done\n");
	else {
		dev_err(lt->dev, "lt8912b_init_config failed\n");
		goto init_err;
	}

	ret = lt8912b_mipi_basic_config(lt);
	if (!ret)
		dev_err(lt->dev, "lt8912b_mipi_basic_config Done\n");
	else {
		dev_err(lt->dev, "lt8912b_mipi_basic_config failed\n");
		goto init_err;
	}

	ret = lt8912b_dds_config(lt);
	if (!ret)
		dev_err(lt->dev, "lt8912b_dds_config Done\n");
	else {
		dev_err(lt->dev, "lt8912b_dds_config failed\n");
		goto init_err;
	}

	ret = lt8912b_parse_video_mode(lt);
	if (!ret)
		dev_err(lt->dev, "lt8912b_parse_video_mode Done\n");
	else {
		dev_err(lt->dev, "lt8912b_parse_video_mode failed\n");
		goto init_err;
	}

	/* Audio Disabled */
	ret = lt8912b_set_audio(lt);
	if (!ret)
		dev_err(lt->dev, "lt8912 Audio disable Done\n");
	else {
		dev_err(lt->dev, "lt8912b Audio disable failed\n");
		goto init_err;
	}

	/* set default avi info frame */
	ret = lt8912b_avi_info_frame(lt);
	if (!ret)
		dev_err(lt->dev, "lt8912b_avi_info_frame Done\n");
	else {
		dev_err(lt->dev, "lt8912b_avi_info_frame failed\n");
		goto init_err;
	}

	ret = lt8912b_rxlogic_dds_reset(lt);
	if (!ret)
		dev_err(lt->dev, "lt8912b_rxlogic_dds_reset Done\n");
	else {
		dev_err(lt->dev, "lt8912b_rxlogic_dds_reset failed\n");
		goto init_err;
	}


	lt8912b_check_dds(lt);

	/* write static lvds config */
	ret = lt8912b_lvds_config_init(lt);
	if (!ret)
		dev_err(lt->dev, "lt8912b_lvds_config_init Done\n");
	else {
		dev_err(lt->dev, "lt8912b_lvds_config_init failed\n");
		goto init_err;
	}

	/* Enable HDMI */
	if (!ret)
		ret = regmap_write(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x33, 0x0e);
	if (!ret)
		dev_err(lt->dev, "Enable HDMI called\n");
	else {
		dev_err(lt->dev, "Enable HDMI failed\n");
		goto init_err;
	}

	/* check the hpd state */
	hpd = lt8912b_get_hpd(lt);
	dev_err(lt->dev, "hpd in init() is : %s\n", hpd ? "high" : "low");

init_err:
	mutex_unlock(&lt->i2c_lock);
}
EXPORT_SYMBOL(lt8912b_init);

static const struct regmap_config lt8912b_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0xff,
};

static int lt8912b_i2c_init(struct lt8912b *lt)
{
	struct i2c_board_info info[] = {
		{ I2C_BOARD_INFO("lt8912p0", LT8912B_I2C_MAIN_ADDR), },
		{ I2C_BOARD_INFO("lt8912p1", LT8912B_I2C_CEC_DSI_ADDR), },
		{ I2C_BOARD_INFO("lt8912p2", LT8912B_I2C_TX_ADDR), }
	};
	unsigned int i;
	int ret;

	if (!lt)
		return -ENODEV;

	for (i = 0; i < ARRAY_SIZE(info); i++) {
		if (i > 0 ) {
			lt->i2c_client[i] = i2c_new_dummy(lt->i2c_client[0]->adapter,
							  info[i].addr);
			if (!lt->i2c_client[i])
				return -ENODEV;
		}
		dev_err(lt->dev,
			"### %s: i2c_init(): %d board info, type: %s, client: %s, addr: %0x\n",
			__func__, i, info[i].type, lt->i2c_client[i]->name,
			lt->i2c_client[i]->addr);

		lt->regmap[i] = devm_regmap_init_i2c(lt->i2c_client[i],
						     &lt8912b_regmap_config);
		if (IS_ERR(lt->regmap[i])) {
			ret = PTR_ERR(lt->regmap[i]);
			dev_err(lt->dev,
				"Failed to initialize regmap: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static int lt8912b_i2c_free(struct lt8912b *lt)
{
	unsigned int i;
	for (i = 1; i < LT8912B_MAX_I2C_CLIENTS; i++)
		i2c_unregister_device(lt->i2c_client[i]);

	return 0;
}

static int lt8912b_of_parse_gpios(struct device *dev)
{
	struct lt8912b *lt = dev_get_drvdata(dev);
	int ret;

	lt->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(lt->reset_gpio)) {
		ret = PTR_ERR(lt->reset_gpio);
		dev_err(dev, "### Failed to get reset gpio; err = %d\n", ret);
		return ret;
	}

	lt->mipi_switch_gpio = devm_gpiod_get(dev, "mipi_switch",
					      GPIOD_OUT_LOW);
	if (IS_ERR(lt->mipi_switch_gpio)) {
		ret = PTR_ERR(lt->mipi_switch_gpio);
		dev_err(dev, "### Failed to get switch gpio; err = %d\n", ret);
		return ret;
	}

	lt->lt8912_irq_gpio = devm_gpiod_get(dev, "lt8912_irq", GPIOD_IN);
	if (IS_ERR(lt->lt8912_irq_gpio)) {
		ret = PTR_ERR(lt->lt8912_irq_gpio);
		dev_err(dev, "### Failed to get lt8912_irq gpio; err = %d\n",
			ret);
		return ret;
	}

	lt->hpd_irq_gpio = devm_gpiod_get(dev, "hpd_irq", GPIOD_IN);
	if (IS_ERR(lt->hpd_irq_gpio)) {
		ret = PTR_ERR(lt->hpd_irq_gpio);
		dev_err(dev, "### Failed to get hpd_irq gpio; err = %d\n", ret);
		return ret;
	}

	return 0;
}

static int lt8912b_setup_irq(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct lt8912b *lt = dev_get_drvdata(dev);
	int ret;

	lt->lt8912_irq = gpiod_to_irq(lt->lt8912_irq_gpio);
	if (lt->lt8912_irq < 0) {
		dev_err(dev, "### Error lt8912 irq: %d\n", lt->lt8912_irq);
		return lt->lt8912_irq;
	}
	client->irq = lt->lt8912_irq;

	lt->hpd_irq = gpiod_to_irq(lt->hpd_irq_gpio);
	if (lt->hpd_irq < 0) {
		dev_err(dev, "### Error hpd irq: %d\n", lt->hpd_irq);
		return lt->hpd_irq;
	}

	ret = devm_request_threaded_irq(&client->dev, client->irq,
			NULL, lt8912b_isr, IRQF_ONESHOT |
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			"lt8912-irq", lt);
	if (ret) {
		dev_err(dev, "### Failed to request LT8912 irq (%d); "
			"err = %d\n", client->irq, ret);
		return ret;
	}

	ret = devm_request_threaded_irq(&client->dev, lt->hpd_irq,
			NULL, lt8912b_hpd_isr, IRQF_ONESHOT |
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			"lt8912-hpd-irq", lt);
	if (ret) {
		dev_err(dev, "### Failed to request HPD irq (%d); ret = %d\n",
			lt->hpd_irq, ret);
		return ret;
	}

	pr_err("### IRQs requested successfully!\n");

	return 0;
}

static int lt8912b_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	int ret;
	unsigned int version[2];

	pr_err("### %s: probe()\n", __func__);

	lt = devm_kzalloc(dev, sizeof(*lt), GFP_KERNEL);
	if (!lt)
		return -ENOMEM;

	dev_set_drvdata(dev, lt);

	lt->i2c_client[0] = client;
	lt->dev = dev;

	// Hard-coding to 1920x1080 to begin with
	lt->vid_mode = &video_1920x1080_60Hz;

	ret = lt8912b_of_parse_gpios(dev);
	if (ret)
		return ret;

	lt8912b_reset(lt);

	/*
	 * TODO: Hard code DSI lines route to HDMI bridge for now.
	 *       More clean solution is to do that HS connector/HDMI muxing in
	 *       lt8912b_hpd_isr().
	 */
	gpiod_set_value(lt->mipi_switch_gpio, 1);

	ret = lt8912b_i2c_init(lt);
	if (ret)
		return ret;

	mutex_init(&lt->i2c_lock);
	i2c_set_clientdata(client, lt);

	/* read chip ID */
	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x00, &version[0]);
	regmap_read(lt->regmap[LT8912B_I2C_MAIN_IDX], 0x01, &version[1]);
	dev_err(lt->dev, "LT8912 ID: %02x, %02x\n", version[0], version[1]);

	ret = lt8912b_setup_irq(client);
	if (ret)
		return ret;

	pr_err("### %s: probe() ended without issues: vid_mode: %0x\n", __func__, lt->vid_mode);
	return 0;
}

static int lt8912b_remove(struct i2c_client *client)
{
	struct lt8912b *lt = i2c_get_clientdata(client);

	lt8912b_i2c_free(lt);
	return 0;
}


int lt8912b_resume(void)
{
	if (!lt || !lt->vid_mode) {
		pr_err("### %s: Error: lt is not initialized\n", __func__);
		return -EINVAL;
	}
	dev_err(lt->dev, "%s: entering\n", __func__);

	lt8912b_reset(lt);

	dev_err(lt->dev, "%s: exiting\n", __func__);
	return 0;
}

int lt8912b_suspend(void)
{
	if (!lt || !lt->vid_mode) {
		pr_err("### %s: Error: lt is not initialized\n", __func__);
		return -EINVAL;
	}
	dev_err(lt->dev, "%s: entering\n", __func__);

	gpiod_direction_output(lt->reset_gpio, 1);
	msleep(120);

	dev_err(lt->dev, "%s: exiting\n", __func__);
	return 0;
}

static const struct i2c_device_id lt8912b_id[] = {
	{ "lt8912b", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, lt8912b_id);

static const struct of_device_id lt8912b_of_match[] = {
	{ .compatible = "lontium,lt8912b" },
	{ }
};
MODULE_DEVICE_TABLE(of, lt8912b_of_match);

static struct i2c_driver lt8912b_driver = {
	.driver = {
		.name		= "lt8912b",
		.of_match_table	= lt8912b_of_match,
	},
	.probe		= lt8912b_probe,
	.remove		= lt8912b_remove,
	.id_table	= lt8912b_id,
};

module_i2c_driver(lt8912b_driver);
