/* SPDX-License-Identifier: GPL-2.0 */
/**
 *
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
 *        http://www.samsung.com
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __PHY_SAMSUNG_USB_FW_CAL_H__
#define __PHY_SAMSUNG_USB_FW_CAL_H__

//#include <lk/list.h>

#define EXYNOS_USBCON_VER_01_0_0 0x0100 /* Istor    */
#define EXYNOS_USBCON_VER_01_0_1 0x0101 /* JF 3.0   */
#define EXYNOS_USBCON_VER_01_1_1 0x0111 /* KC       */
#define EXYNOS_USBCON_VER_01_MAX 0x01FF

#define EXYNOS_USBCON_VER_02_0_0 0x0200 /* Insel-D, Island  */
#define EXYNOS_USBCON_VER_02_0_1 0x0201 /* JF EVT0 2.0 Host */
#define EXYNOS_USBCON_VER_02_1_0 0x0210
#define EXYNOS_USBCON_VER_02_1_1 0x0211 /* JF EVT1 2.0 Host */
#define EXYNOS_USBCON_VER_02_1_2 0x0212 /* Katmai EVT0 */
#define EXYNOS_USBCON_VER_02_MAX 0x02FF

#define EXYNOS_USBCON_VER_03_0_0 0x0300 /* Lhotse, Lassen HS */
#define EXYNOS_USBCON_VER_03_0_1 0x0301 /* Super Speed          */
#define EXYNOS_USBCON_VER_03_MAX 0x03FF

/* Samsung phy */
#define EXYNOS_USBCON_VER_04_0_0 0x0400 /* Exynos 9810  */
#define EXYNOS_USBCON_VER_04_0_1 0x0401 /* Exynos 9820  */
#define EXYNOS_USBCON_VER_04_0_2 0x0402 /* Exynos 9830 */
#define EXYNOS_USBCON_VER_04_0_3 0x0403 /* Exynos 9630  */
#define EXYNOS_USBCON_VER_04_0_4 0x0404 /* Exynos 9840  */
#define EXYNOS_USBCON_VER_04_MAX 0x04FF

/* Sub phy control - not include System/Link control */
#define EXYNOS_USBCON_VER_05_0_0 0x0500 /* High Speed Only  */
#define EXYNOS_USBCON_VER_05_1_0 0x0510 /* Super Speed      */
#define EXYNOS_USBCON_VER_05_3_0 0x0530 /* Super Speed Dual PHY */
#define EXYNOS_USBCON_VER_05_MAX 0x05FF

/* block control version */
#define EXYNOS_USBCON_VER_06_0_0 0x0600 /* link control only */
#define EXYNOS_USBCON_VER_06_4_0 0x0610 /* link + usb2.0 phy */
#define EXYNOS_USBCON_VER_06_2_0 0x0620 /* link + usb3.0 phy */
#define EXYNOS_USBCON_VER_06_3_0 0x0630 /* link + usb2.0 + usb3.0 phy */
#define EXYNOS_USBCON_VER_06_MAX 0x06FF

/* eUSB phy contorller */
#define EXYNOS_USBCON_VER_07_0_0 0x0700 /* eUSB PHY controller */
#define EXYNOS_USBCON_VER_07_8_0 0x0780 /* dwc eUSB PHY register interface */

/* synopsys usbdp phy contorller */
#define EXYNOS_USBCON_VER_08_0_0 0x0800 /* dwc usb3p2/dp PHY controller */

#define EXYNOS_USBCON_VER_F2_0_0 0xF200
#define EXYNOS_USBCON_VER_F2_MAX 0xF2FF

#define EXYNOS_USBCON_VER_MAJOR_VER_MASK 0xFF00
#define EXYNOS_USBCON_VER_SS_ONLY_CAP 0x0010
#define EXYNOS_USBCON_VER_SS_CAP 0x0040
#define EXYNOS_USBCON_VER_SS_HS_CAP 0x0080
#define EXYNOS_USBCON_VER_MINOR(_x) ((_x) &0xf)
#define EXYNOS_USBCON_VER_MID(_x) ((_x) &0xf0)
#define EXYNOS_USBCON_VER_MAJOR(_x) ((_x) &0xff00)

#define EXYNOS_BLKCON_VER_HS_CAP 0x0010
#define EXYNOS_BLKCON_VER_SS_CAP 0x0020

enum exynos_usbphy_mode {
	USBPHY_MODE_DEV = 0,
	USBPHY_MODE_HOST = 1,

	/* usb phy for uart bypass mode */
	USBPHY_MODE_BYPASS = 0x10,
};

// typedef unsigned char unsigned char;
#ifndef __iomem
#define __iomem
#endif

enum exynos_usbphy_refclk {
	USBPHY_REFCLK_DIFF_100MHZ = 0x80 | 0x27,
	USBPHY_REFCLK_DIFF_52MHZ = 0x80 | 0x02 | 0x40,
	USBPHY_REFCLK_DIFF_48MHZ = 0x80 | 0x2a | 0x40,
	USBPHY_REFCLK_DIFF_26MHZ = 0x80 | 0x02,
	USBPHY_REFCLK_DIFF_24MHZ = 0x80 | 0x2a,
	USBPHY_REFCLK_DIFF_20MHZ = 0x80 | 0x31,
	USBPHY_REFCLK_DIFF_19_2MHZ = 0x80 | 0x38,

	USBPHY_REFCLK_EXT_50MHZ = 0x07,
	USBPHY_REFCLK_EXT_48MHZ = 0x08,
	USBPHY_REFCLK_EXT_26MHZ = 0x06,
	USBPHY_REFCLK_EXT_24MHZ = 0x05,
	USBPHY_REFCLK_EXT_20MHZ = 0x04,
	USBPHY_REFCLK_EXT_19P2MHZ = 0x01,
	USBPHY_REFCLK_EXT_12MHZ = 0x02,
};

enum exynos_usbphy_utmi {
	USBPHY_UTMI_FREECLOCK,
	USBPHY_UTMI_PHYCLOCK,
};

enum exynos_usb_bc {
	BC_NO_CHARGER,
	BC_SDP,
	BC_DCP,
	BC_CDP,
	BC_ACA_DOCK,
	BC_ACA_A,
	BC_ACA_B,
	BC_ACA_C,
};

#define EXYNOS_USB_TUNE_LAST 0x4C415354

/**
 * struct exynos_usbphy_info : USBPHY information to share USBPHY CAL code
 * @version: PHY controller version
 *       0x0100 - for EXYNOS_USB3 : EXYNOS7420, EXYNOS7890
 *       0x0101 -           EXYNOS8890
 *       0x0111 -           EXYNOS8895
 *       0x0200 - for EXYNOS_USB2 : EXYNOS7580, EXYNOS3475
 *       0x0210 -           EXYNOS8890_EVT1
 *       0xF200 - for EXT         : EXYNOS7420_HSIC
 * @refclk: reference clock frequency for USBPHY
 * @refsrc: reference clock source path for USBPHY
 * @use_io_for_ovc: use over-current notification io for USBLINK
 * @regs_base: base address of PHY control register *
 */

struct exynos_usbphy_info {
	/* Device Information */
	struct device *dev;

	u32 version;
	enum exynos_usbphy_refclk refclk;

	bool use_io_for_ovc;
	bool common_block_disable;
	bool not_used_vbus_pad;

	void __iomem *regs_base;

	/* multiple phy */
	int hw_version;
	void __iomem *regs_base_2nd;
	void __iomem *pma_base;
	void __iomem *pcs_base;
	void __iomem *ctrl_base;
	void __iomem *link_base;
	int used_phy_port;

	/* Alternative PHY REF_CLK source */
	bool alt_ref_clk;

	/* Dual PHY */
	bool dual_phy;
};

struct usb_eom_result_s {
	u32 phase;
	u32 vref;
	u64 err;
};
#define EOM_PH_SEL_MAX 72
#define EOM_DEF_VREF_MAX 256

void phy_usb_exynos_register_cal_infor(struct exynos_usbphy_info *cal_info);

#endif /* __PHY_SAMSUNG_USB_FW_CAL_H__ */
