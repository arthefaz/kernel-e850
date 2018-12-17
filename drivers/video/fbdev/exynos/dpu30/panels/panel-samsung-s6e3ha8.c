/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Samsung S6E3HA8 Panel driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <video/mipi_display.h>
#include "exynos_panel_drv.h"
#include "s6e3ha8_param.h"
#include "../dsim.h"

static int s6e3ha8_suspend(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha8_displayon(struct exynos_panel_device *panel)
{
	int id = 0;
	struct decon_lcd *lcd = &panel->lcd_info;

	DPU_INFO_PANEL("%s +\n", __func__);

	msleep(5);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_F0,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_F0)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_ON_F0 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TEST_KEY_ON_FC,
				ARRAY_SIZE(SEQ_TEST_KEY_ON_FC)) < 0)
		dsim_err("fail to write SEQ_TEST_KEY_ON_FC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DSC_PRA, (unsigned long)SEQ_DSC_EN[0], 0) < 0)
		dsim_err("fail to write SEQ_DSC_EN command.\n");

	switch (lcd->dsc_slice_num) {
		case 2:
			if (dsim_wr_data(id, MIPI_DSI_DSC_PPS, (unsigned long)SEQ_PPS_SLICE2,
						ARRAY_SIZE(SEQ_PPS_SLICE2)) < 0)
				dsim_err("fail to write SEQ_PPS_SLICE2 command.\n");
			break;
		default:
			dsim_err("fail to set MIPI_DSI_DSC_PPS command(no slice).\n");
			break;
	}

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_SLEEP_OUT[0], 0) < 0)
		dsim_err("fail to send SEQ_SLEEP_OUT command.\n");

	msleep(120);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_TSP_HSYNC,
				ARRAY_SIZE(SEQ_TSP_HSYNC)) < 0)
		dsim_err("fail to write SEQ_TSP_HSYNC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_SET_AREA,
				ARRAY_SIZE(SEQ_SET_AREA)) < 0)
		dsim_err("fail to write SEQ_SET_AREA command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)GAMCTL1,
				ARRAY_SIZE(GAMCTL1)) < 0)
		dsim_err("fail to write GAMCTL1 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)GAMCTL2,
				ARRAY_SIZE(GAMCTL2)) < 0)
		dsim_err("fail to write GAMCTL2 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)GAMCTL3,
				ARRAY_SIZE(GAMCTL3)) < 0)
		dsim_err("fail to write GAMCTL3 command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)BCCTL,
				ARRAY_SIZE(BCCTL)) < 0)
		dsim_err("fail to write BCCTL command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				(unsigned long)PANEL_UPDATE[0],
				(u32)PANEL_UPDATE[1]) < 0)
		dsim_err("fail to send PANEL_UPDATE command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				(unsigned long)SEQ_B_CTRL_ON[0],
				(u32)SEQ_B_CTRL_ON[1]) < 0)
		dsim_err("fail to send B_CTRL_ON command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_TE_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_TE_ON command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				(unsigned long)SEQ_ERR_FG[0],
				(u32)SEQ_ERR_FG[1]) < 0)
		dsim_err("fail to send SEQ_ERR_FG command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SEQ_TE_START_SETTING,
				ARRAY_SIZE(SEQ_TE_START_SETTING)) < 0)
		dsim_err("fail to write SEQ_TE_START_SETTING command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)SEQ_FFC,
				ARRAY_SIZE(SEQ_FFC)) < 0)
		dsim_err("fail to write SEQ_FFC command.\n");

	if (dsim_wr_data(id, MIPI_DSI_DCS_SHORT_WRITE,
				(unsigned long)SEQ_DISPLAY_ON[0], 0) < 0)
		dsim_err("fail to send SEQ_DISPLAY_ON command.\n");

	DPU_INFO_PANEL("%s -\n", __func__);

	return 0;
}

static int s6e3ha8_mres(struct exynos_panel_device *panel, int mres_idx)
{
	int id = 0;
	int dsc_en;
	dsc_en = panel->lcd_info.dt_lcd_mres.res_info[mres_idx].dsc_en;

	DPU_INFO_PANEL("%s +\n", __func__);

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)KEY1_ENABLE,
				ARRAY_SIZE(KEY1_ENABLE)) < 0) {
		dsim_err("failed to write KEY1_ENABLE\n");
	}
	if (dsc_en) {
		if (dsim_wr_data(id, MIPI_DSI_DSC_PRA,
					(unsigned long)DSC_EN[1][0], 0) < 0) {
			dsim_err("failed to write DSC_EN\n");
		}
		if (dsim_wr_data(id, MIPI_DSI_DSC_PPS,
					(unsigned long)PPS_TABLE[mres_idx],
					ARRAY_SIZE(PPS_TABLE[mres_idx])) < 0) {
			dsim_err("failed to write PPS_TABLE[%d]\n", mres_idx);
		}
	} else {
		if (dsim_wr_data(id, MIPI_DSI_DSC_PRA,
					(unsigned long)DSC_EN[0][0], 0) < 0) {
			dsim_err("failed to write DSC_EN\n");
		}
	}

	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)KEY1_DISABLE,
				ARRAY_SIZE(KEY1_DISABLE)) < 0) {
		dsim_err("failed to write KEY1_DISABLE\n");
	}
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)CASET_TABLE[mres_idx],
				ARRAY_SIZE(CASET_TABLE[mres_idx])) < 0) {
		dsim_err("failed to write CASET_TABLE[%d]\n", mres_idx);
	}
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)PASET_TABLE[mres_idx],
				ARRAY_SIZE(PASET_TABLE[mres_idx])) < 0) {
		dsim_err("failed to write PASET_TABLE[%d]\n", mres_idx);
	}
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)KEY2_ENABLE,
				ARRAY_SIZE(KEY2_ENABLE)) < 0) {
		dsim_err("failed to write KEY2_ENABLE\n");
	}
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE,
				(unsigned long)SCALER_TABLE[mres_idx],
				ARRAY_SIZE(SCALER_TABLE[mres_idx])) < 0) {
		dsim_err("failed to write SCALER_TABLE[%d]\n", mres_idx);
	}
	if (dsim_wr_data(id, MIPI_DSI_DCS_LONG_WRITE, (unsigned long)KEY2_DISABLE,
				ARRAY_SIZE(KEY2_DISABLE)) < 0) {
		dsim_err("failed to write KEY2_DISABLE\n");
	}

	DPU_INFO_PANEL("%s -\n", __func__);

	return 0;
}

static int s6e3ha8_doze(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha8_doze_suspend(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha8_dump(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

static int s6e3ha8_read_state(struct exynos_panel_device *panel)
{
	DPU_INFO_PANEL("%s +\n", __func__);
	DPU_INFO_PANEL("%s -\n", __func__);
	return 0;
}

struct exynos_panel_ops panel_s6e3ha8_ops = {
	.id		= 0x460091,
	.suspend	= s6e3ha8_suspend,
	.displayon	= s6e3ha8_displayon,
	.mres		= s6e3ha8_mres,
	.doze		= s6e3ha8_doze,
	.doze_suspend	= s6e3ha8_doze_suspend,
	.dump		= s6e3ha8_dump,
	.read_state	= s6e3ha8_read_state,
};
