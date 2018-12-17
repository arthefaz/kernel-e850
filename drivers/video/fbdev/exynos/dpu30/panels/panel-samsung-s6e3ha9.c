/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Samsung S6E3HA9 Panel driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <video/mipi_display.h>
#include "exynos_panel_drv.h"
#include "s6e3ha8_param.h"
#include "../dsim.h"

static int s6e3ha9_suspend(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha9_displayon(struct exynos_panel_device *panel)
{
	int id = 0;
	struct decon_lcd *lcd = &panel->lcd_info;

	msleep(5);

	DPU_INFO_PANEL("%s +\n", __func__);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_F0_HA9,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_F0_HA9)) < 0)
		DPU_ERR_PANEL("fail to write SEQ_TEST_KEY_ON_F0 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_FC_HA9,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_FC_HA9)) < 0)
		DPU_ERR_PANEL("fail to write SEQ_TEST_KEY_ON_FC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DSC_PRA, (unsigned long)SEQ_DSC_EN_HA9[0], 0) < 0)
		DPU_ERR_PANEL("fail to write SEQ_DSC_EN command.\n");

	switch (lcd->dsc_slice_num) {
		case 2:
			if (dsim_wr_data(id, MIPI_DSI_DSC_PPS, (unsigned long)SEQ_PPS_SLICE2_HA9,
						ARRAY_SIZE(SEQ_PPS_SLICE2_HA9)) < 0)
				DPU_ERR_PANEL("fail to write SEQ_PPS_SLICE2 command.\n");
			break;
		default:
			DPU_ERR_PANEL("fail to set MIPI_DSI_DSC_PPS command(no slice).\n");
			break;
	}

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_SLEEP_OUT_HA9[0], 0) < 0)
		DPU_ERR_PANEL("fail to send SEQ_SLEEP_OUT command.\n");

	msleep(120);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TSP_HSYNC_HA9,
				ARRAY_SIZE(SEQ_TSP_HSYNC_HA9)) < 0)
		DPU_ERR_PANEL("fail to write SEQ_TSP_HSYNC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_TE_ON_HA9[0], 0) < 0)
		DPU_ERR_PANEL("fail to send SEQ_TE_ON command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				(unsigned long)SEQ_ERR_FG_HA9[0], (u32)SEQ_ERR_FG_HA9[1]) < 0)
		DPU_ERR_PANEL("fail to send SEQ_ERR_FG command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TE_START_SETTING_HA9,
				ARRAY_SIZE(SEQ_TE_START_SETTING_HA9)) < 0)
		DPU_ERR_PANEL("fail to write SEQ_TE_START_SETTING command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_FFC_HA9,
				ARRAY_SIZE(SEQ_FFC_HA9)) < 0)
		DPU_ERR_PANEL("fail to write SEQ_FFC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_DISPLAY_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");

	DPU_INFO_PANEL("%s -\n", __func__);

	return 0;
}

static int s6e3ha9_mres(struct exynos_panel_device *panel, int mres_idx)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha9_doze(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha9_doze_suspend(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha9_dump(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha9_read_state(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

struct exynos_panel_ops panel_s6e3ha9_ops = {
	.id		= 0x001090,
	.suspend	= s6e3ha9_suspend,
	.displayon	= s6e3ha9_displayon,
	.mres		= s6e3ha9_mres,
	.doze		= s6e3ha9_doze,
	.doze_suspend	= s6e3ha9_doze_suspend,
	.dump		= s6e3ha9_dump,
	.read_state	= s6e3ha9_read_state,
};
