/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Samsung Dummy Panel driver for lt8912b HDMI bridge.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <video/mipi_display.h>
#include "exynos_panel_drv.h"
#include "../dsim.h"


extern lt8912b_init(void);
extern int lt8912b_resume(void);
extern int lt8912b_suspend(void);

static int lt8912b_dummy_suspend(struct exynos_panel_device *panel)
{

    DPU_INFO_PANEL("%s +\n", __func__);
	lt8912b_suspend();
	return 0;
}

static int lt8912b_dummy_displayon(struct exynos_panel_device *panel)
{
	struct dsim_device *dsim = get_dsim_drvdata(0);
	u8 data[2];

	DPU_INFO_PANEL("%s +\n", __func__);
	lt8912b_resume();

	data[0] = 0x11;
	data[1] = 0x00;
	//SET MIPI CMD enable
	if (dsim_wr_data(dsim, MIPI_DSI_DCS_SHORT_WRITE_PARAM, data, 2, 1) < 0)
		dsim_err("fail to send 0x11 display on command.\n");

	mdelay(10);
	data[0] = 0x29;
	data[1] = 0x00;
	//SET MIPI CMD enable
	if (dsim_wr_data(dsim, MIPI_DSI_DCS_SHORT_WRITE_PARAM, data, 2, 1) < 0)
		dsim_err("fail to send 0x11 display on command.\n");
	mdelay(10);
	lt8912b_init();
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int lt8912b_dummy_mres(struct exynos_panel_device *panel, int mres_idx)
{
    DPU_INFO_PANEL("%s \n", __func__);
	return 0;
}

static int lt8912b_dummy_doze(struct exynos_panel_device *panel)
{
	return 0;
}

static int lt8912b_dummy_doze_suspend(struct exynos_panel_device *panel)
{
	return 0;
}

static int lt8912b_dummy_dump(struct exynos_panel_device *panel)
{
	return 0;
}

static int lt8912b_dummy_read_state(struct exynos_panel_device *panel)
{
	return 0;
}

static int lt8912b_dummy_set_light(struct exynos_panel_device *panel, u32 br_val)
{

	DPU_INFO_PANEL("%s +\n", __func__);

	DPU_INFO_PANEL("requested brightness value = %d\n", br_val);

	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}
struct exynos_panel_ops panel_lt8912b_dummy_ops = {
	.id		= {0x0000ff},
	.suspend	= lt8912b_dummy_suspend,
	.displayon	= lt8912b_dummy_displayon,
	.mres		= lt8912b_dummy_mres,
	.doze		= lt8912b_dummy_doze,
	.doze_suspend	= lt8912b_dummy_doze_suspend,
	.dump		= lt8912b_dummy_dump,
	.read_state	= lt8912b_dummy_read_state,
	.set_light	= lt8912b_dummy_set_light,
};
