/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Samsung S6E3FA0 Panel driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <video/mipi_display.h>
#include "exynos_panel_drv.h"
#include "../dsim.h"

int s6e3fa0_suspend(struct exynos_panel_device *panel)
{
	struct dsim_device *dsim = get_dsim_drvdata(0);

	dsim_write_data_seq_delay(dsim, 20, 0x28, 0x00, 0x00);

	return 0;
}

int s6e3fa0_displayon(struct exynos_panel_device *panel)
{
	struct exynos_panel_info *lcd = &panel->lcd_info;
	struct dsim_device *dsim = get_dsim_drvdata(0);

	dsim_write_data_seq_delay(dsim, 12, 0xF0, 0x5A, 0x5A);
	dsim_write_data_seq_delay(dsim, 12, 0xF1, 0x5A, 0x5A);
	dsim_write_data_seq_delay(dsim, 12, 0xFC, 0x5A, 0x5A);
	dsim_write_data_seq_delay(dsim, 12, 0xED, 0x01, 0x00);

	if (lcd->mode == DECON_MIPI_COMMAND_MODE) {
		dsim_write_data_seq_delay(dsim, 12, 0xFD, 0x16, 0x80);
		dsim_write_data_seq_delay(dsim, 12, 0xF6, 0x08);
	} else {
		dsim_write_data_seq_delay(dsim, 120, 0xE7, 0xED, 0xC7, 0x23,
				0x57, 0xA5);
	}

	dsim_write_data_seq_delay(dsim, 20, 0x11); /* sleep out */

	if (lcd->mode == DECON_VIDEO_MODE)
		dsim_write_data_seq_delay(dsim, 12, 0xF2, 0x02);

	dsim_write_data_seq_delay(dsim, 12, 0xEB, 0x01, 0x00);
	dsim_write_data_seq_delay(dsim, 12, 0xC0, 0x63, 0x02, 0x03, 0x32, 0xFF,
			0x44, 0x44, 0xC0, 0x00, 0x40);

	/* enable brightness control */
	dsim_write_data_seq_delay(dsim, 12, 0x53, 0x20, 0x00);

	if (lcd->mode == DECON_MIPI_COMMAND_MODE)
		dsim_write_data_seq_delay(dsim, 12, 0x35); /* TE on */

	if (lcd->mode == DECON_VIDEO_MODE)
		mdelay(120);

	dsim_write_data_seq(dsim, 0x29); /* display on */

	return 0;
}

int s6e3fa0_mres(struct exynos_panel_device *panel, int mres_idx)
{
	return 0;
}

int s6e3fa0_doze(struct exynos_panel_device *panel)
{
	return 0;
}

int s6e3fa0_doze_suspend(struct exynos_panel_device *panel)
{
	return 0;
}

int s6e3fa0_dump(struct exynos_panel_device *panel)
{
	return 0;
}

int s6e3fa0_read_state(struct exynos_panel_device *panel)
{
	return 0;
}

struct exynos_panel_ops panel_s6e3fa0_ops = {
	.id		= 0x244040,
	.suspend	= s6e3fa0_suspend,
	.displayon	= s6e3fa0_displayon,
	.mres		= s6e3fa0_mres,
	.doze		= s6e3fa0_doze,
	.doze_suspend	= s6e3fa0_doze_suspend,
	.dump		= s6e3fa0_dump,
	.read_state	= s6e3fa0_read_state,
};
