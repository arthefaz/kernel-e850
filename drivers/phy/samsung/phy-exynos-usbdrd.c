// SPDX-License-Identifier: GPL-2.0
/*
 * Samsung EXYNOS SoC series USB DRD PHY driver
 *
 * Phy provider for USB 3.0 DRD controller on Exynos SoC series
 *
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
 *        http://www.samsung.com
 *
 * Author: Vivek Gautam <gautam.vivek@samsung.com>
 *	   Minho Lee <minho55.lee@samsung.com>
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

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/usb/otg.h>
#ifdef CONFIG_OF
#include <linux/of_device.h>
#endif

/* ---- Registers ----------------------------------------------------------- */

#define EXYNOS_USBCON_CTRL_VER		(0x00)

#define EXYNOS_USBCON_LINK_CTRL		(0x04)
#define LINKCTRL_PIPE3_FORCE_RX_ELEC_IDLE	(0x1 << 18)
#define LINKCTRL_PIPE3_FORCE_PHY_STATUS		(0x1 << 17)
#define LINKCTRL_PIPE3_FORCE_EN			(0x1 << 16)
#define LINKCTRL_DIS_QACT_BUSPEND		(0x1 << 13)
#define LINKCTRL_DIS_QACT_LINKGATE		(0x1 << 12)
#define LINKCTRL_DIS_QACT_ID0			(0x1 << 11)
#define LINKCTRL_DIS_QACT_VBUS_VALID		(0x1 << 10)
#define LINKCTRL_DIS_QACT_BVALID		(0x1 << 9)
#define LINKCTRL_FORCE_QACT			(0x1 << 8)
#define LINKCTRL_BUS_FILTER_BYPASS(_x)		((_x & 0xf) << 4)
#define LINKCTRL_BUS_FILTER_BYPASS_MASK		(0xf << 4)
#define LINKCTRL_HOST_SYSTEM_ERR		(0x1 << 2)
#define LINKCTRL_LINK_PME			(0x1 << 1)
#define LINKCTRL_PME_GENERATION			(0x1 << 0)

#define EXYNOS_USBCON_LINK_PORT		(0x08)
#define LINKPORT_HOST_NUM_U3(_x)		((_x & 0xf) << 16)
#define LINKPORT_HOST_NUM_U2(_x)		((_x & 0xf) << 12)
#define LINKPORT_HOST_U3_PORT_DISABLE		(0x1 << 9)
#define LINKPORT_HOST_U2_PORT_DISABLE		(0x1 << 8)
#define LINKPORT_HOST_PORT_POWER_CON_PRESENT	(0x1 << 6)
#define LINKPORT_HUB_PORT_SET_OCD_U3		(0x1 << 5)
#define LINKPORT_HUB_PORT_SET_OCD_U2		(0x1 << 4)
#define LINKPORT_HUB_PORT_SEL_OCD_U3		(0x1 << 3)
#define LINKPORT_HUB_PORT_SEL_OCD_U2		(0x1 << 2)
#define LINKPORT_HUB_PERM_ATTACH_U3		(0x1 << 1)
#define LINKPORT_HUB_PERM_ATTACH_U2		(0x1 << 0)

#define EXYNOS_USBCON_LINK_DEBUG_L	(0x0C)
#define EXYNOS_USBCON_LINK_DEBUG_H	(0x10)

#define EXYNOS_USBCON_LTSTATE_HIS	(0x14)
#define LTSTATE_LINKTRN_DONE			(0x1 << 31)
#define LTSTATE_HIS4(_x)			((_x & 0xf) << 16)
#define LTSTATE_HIS3(_x)			((_x & 0xf) << 12)
#define LTSTATE_HIS2(_x)			((_x & 0xf) << 8)
#define LTSTATE_HIS1(_x)			((_x & 0xf) << 4)
#define LTSTATE_HIS0(_x)			((_x & 0xf) << 0)

#define EXYNOS_USBCON_CLKRST		(0x20)
#define CLKRST_PHY20_SW_RST			(0x1 << 13)
#define CLKRST_PHY20_RST_SEL			(0x1 << 12)
#define CLKRST_USBAUDIO_CLK_GATE_EN		(0x1 << 9)
#define CLKRST_USBAUDIO_CLK_SEL			(0x1 << 8)
#define CLKRST_LINK_PCLK_SEL			(0x1 << 7)
#define CLKRST_PHYCLOCKSEL			(0x1 << 6)
#define CLKRST_REFCLK_SEL			(0x1 << 4)
/* MK Verion */
#define CLKRST_PHY30_SW_RST			(0x1 << 3)
#define CLKRST_PHY30_RST_SEL			(0x1 << 2)
/* Lhotse Verion */
#define CLKRST_PHY_SW_RST			(0x1 << 3)
#define CLKRST_PHY_RST_SEL			(0x1 << 2)
#define CLKRST_PORT_RST				(0x1 << 1)
#define CLKRST_LINK_SW_RST			(0x1 << 0)

#define EXYNOS_USBCON_PWR		(0x24)

#define PWR_FORCE_POWERDOWN_EN			(0x1 << 3)
#define RSVD1					(0x7 << 0)
#define PWR_FORCE_POWERDONW			(0x1 << 2)
#define PWR_TEST_POWERDOWN_SSP			(0x1 << 1)
#define PWR_TEST_POWERDOWN_HSP			(0x1 << 0)

#define EXYNOS_USBCON_DUALPHYSEL	(0x28)
#define DUALPHYSEL_PHYSEL_CTRL			(0x1 << 0)
#define DUALPHYSEL_PHYSEL_SSPHY			(0x1 << 1)
#define DUALPHYSEL_PHYSEL_PIPECLK		(0x1 << 4)
#define DUALPHYSEL_PHYSEL_PIPERST		(0x1 << 8)

#define EXYNOS_USBCON_SSP_PLL		(0x30)
#define SSP_PLL_MPLL_MULTIPLIER_MASK		(0x7F << 24)
#define SSP_PLL_MPLL_MULTIPLIER(_x)		((_x & 0x7f) << 24)
#define SSP_PLL_SSC_REF_CLK_SEL_MASK		(0x1ff << 12)
#define SSP_PLL_SSC_REF_CLK_SEL(_x)		((_x & 0x1ff) << 12)
#define SSP_PLL_SSC_EN				(0x1 << 11)
#define SSP_PLL_SSC_RANGE_MASK			(0x7 << 8)
#define SSP_PLL_SSC_RANGE(_x)			((_x & 0x7) << 8)
#define SSP_PLL_REF_SSP_EN			(0x1 << 7)
#define SSP_PLL_REF_CLKDIV2			(0x1 << 6)
#define SSP_PLL_FSEL_MASK			(0x3f << 0)
#define SSP_PLL_FSEL(_x)			((_x & 0x3f) << 0)

#define EXYNOS_USBCON_SSP_PARACON0	(0x34)
#define SSP_PARACON0_TX0_TERM_OFFSET_MASK	(0x1f << 25)
#define SSP_PARACON0_TX0_TERM_OFFSET(_x)	((_x & 0x1f) << 25)
#define SSP_PARACON0_PCS_TX_SWING_FULL_MASK	(0x7f << 16)
#define SSP_PARACON0_PCS_TX_SWING_FULL(_x)	((_x & 0x7f) << 16)
#define SSP_PARACON0_PCS_TX_DEEMPH_6DB_MASK	(0x3f << 8)
#define SSP_PARACON0_PCS_TX_DEEMPH_6DB(_x)	((_x & 0x3f) << 8)
#define SSP_PARACON0_PCS_TX_DEEMPH_3P5DB_MASK	(0x3f << 0)
#define SSP_PARACON0_PCS_TX_DEEMPH_3P5DB(_x)	((_x & 0x3f) << 0)

#define EXYNOS_USBCON_SSP_PARACON1	(0x38)
#define SSP_PARACON1_TX_VBOOST_LVL_SSTX_MASK	(0x7 << 28)
#define SSP_PARACON1_TX_VBOOST_LVL_SSTX(_x)		((_x & 0x7) << 28)
#define SSP_PARACON1_TX_VBOOST_LVL_MASK		(0x7 << 24)
#define SSP_PARACON1_TX_VBOOST_LVL(_x)		((_x & 0x7) << 24)
#define SSP_PARACON1_LOS_LEVEL_MASK		(0x1f << 16)
#define SSP_PARACON1_LOS_LEVEL(_x)		((_x & 0x1f) << 16)
#define SSP_PARACON1_LOS_BIAS_MASK		(0x7 << 12)
#define SSP_PARACON1_LOS_BIAS(_x)		((_x & 0x7) << 12)
#define SSP_PARACON1_PCS_RX_LOS_MASK_VAL_MASK	(0x3ff << 0)
#define SSP_PARACON1_PCS_RX_LOS_MASK_VAL(_x)	((_x & 0x3ff) << 0)

#define EXYNOS_USBCON_SSP_TEST		(0x3C)
#define SSP_TEST_TX_EYE_HEIGHT_CNTL_EN_MASK		(0x1 << 28)
#define SSP_TEST_TX_EYE_HEIGHT_CNTL_EN(_x)		((_x & 0x1) << 28)
#define SSP_TEST_PIPE_TX_DEEMPH_UPDATE_DELAY_MASK	(0xf << 24)
#define SSP_TEST_PIPE_TX_DEEMPH_UPDATE_DELAY(_x)	((_x & 0xf) << 24)
#define SSP_TEST_PCS_TX_SWING_FULL_SSTX_MASK	(0x7f << 16)
#define SSP_TEST_PCS_TX_SWING_FULL_SSTX(_x)		((_x & 0x7f) << 16)
#define SSP_TEST_RTUNE_ACK			(0x1 << 3)
#define SSP_TEST_RTUNE_REQ			(0x1 << 2)
#define SSP_TEST_LANE0_TX2RX_LOOPBK		(0x1 << 1)
#define SSP_TEST_LOOPBACKENB			(0x1 << 0)

#define EXYNOS_USBCON_SSP_CRCTL0	(0x40)
#define SSP_CCTRL0_CR_DATA_IN_MASK		(0xffffU << 16)
#define SSP_CCTRL0_CR_DATA_IN(_x)		((_x & 0xffffU) << 16)
#define SSP_CRCTRL0_CR_WRITE			(0x1 << 3)
#define SSP_CRCTRL0_CR_READ			(0x1 << 2)
#define SSP_CRCTRL0_CR_CAP_DATA			(0x1 << 1)
#define SSP_CRCTRL0_CR_CAP_ADDR			(0x1 << 0)

#define EXYNOS_USBCON_SSP_CRCTL1	(0x44)
#define SSP_CRCTL1_CR_DATA_OUT_MASK		(0xffffU << 16)
#define SSP_CRCTL1_CR_DATA_OUT(_x)		((_x & 0xffffU) << 16)
#define SSP_CRCTL1_CR_ACK			(0x1 << 0)

#define EXYNOS_USBCON_COMBO_PMA_CTRL	(0x48)
/* S5E9820 added */
#define PMA_REF_SOC_PLL_SSC             (0x1 << 16)
#define PMA_ROPLL_REF_CLK_SEL_MASK      (0x3 << 12)
#define PMA_ROPLL_REF_CLK_SEL_SET(_x)   ((_x & 0x3) << 12)
#define PMA_ROPLL_REF_CLK_SEL_GET(_x)   ((_x & (0x3 << 12)) >> 12)
#define PMA_LCPLL_REF_CLK_SEL_MASK      (0x3 << 10)
#define PMA_LCPLL_REF_CLK_SEL_SET(_x)   ((_x & 0x3) << 10)
#define PMA_LCPLL_REF_CLK_SEL_GET(_x)   ((_x & (0x3 << 10)) >> 10)
/* S5E9820 added */
#define PMA_PLL_REF_REQ_MASK			(0x3 << 10)
#define PMA_PLL_REF_REQ_SET(_x)			((_x & 0x3) << 10)
#define PMA_PLL_REF_REQ_GET(_x)			((_x & (0x3 << 10)) >> 10)
#define PMA_REF_FREQ_SEL_MASK			(0x3 << 8)
#define PMA_REF_FREQ_SEL_SET(_x)			((_x & 0x3) << 8)
#define PMA_REF_FREQ_SEL_GET(_x)			((_x & (0x3 << 8)) >> 8)
#define PMA_LOW_PWR					(0x1 << 4)
#define PMA_TRSV_SW_RST				(0x1 << 3)
#define PMA_CMN_SW_RST				(0x1 << 2)
#define PMA_INIT_SW_RST				(0x1 << 1)
#define PMA_APB_SW_RST				(0x1 << 0)


#define EXYNOS_USBCON_UTMI		(0x50)
#define UTMI_OPMODE_CTRL_EN			(0x1 << 8)
#define UTMI_FORCE_OPMODE_MASK			(0x3 << 6)
#define UTMI_FORCE_OPMODE_SET(_x)		((_x & 0x3) << 6)
#define UTMI_FORCE_VBUSVALID			(0x1 << 5)
#define UTMI_FORCE_BVALID			(0x1 << 4)
#define UTMI_DP_PULLDOWN			(0x1 << 3)
#define UTMI_DM_PULLDOWN			(0x1 << 2)
#define UTMI_FORCE_SUSPEND			(0x1 << 1)
#define UTMI_FORCE_SLEEP			(0x1 << 0)

#define EXYNOS_USBCON_HSP		(0x54)
#define HSP_AUTORSM_ENB				(0x1 << 29)
#define HSP_RETENABLE_EN			(0x1 << 28)
#define HSP_FSVM_OUT_EN				(0x1 << 26)
#define HSP_FSLS_SPEED_SEL			(0x1 << 25)
#define HSP_FSVP_OUT_EN				(0x1 << 24)
#define HSP_HS_XCVR_EXT_CTL			(0x1 << 22)
#define HSP_HS_RXDAT				(0x1 << 21)
#define HSP_HS_SQUELCH				(0x1 << 20)
#define HSP_FSVMINUS				(0x1 << 17)
#define HSP_FSVMINUS_GET(_x)		((_x & (0x1 << 17)) >> 17)
#define HSP_FSVPLUS				(0x1 << 16)
#define HSP_FSVPLUS_GET(_x)		((_x & (0x1 << 16)) >> 16)
#define HSP_VBUSVLDEXTSEL			(0x1 << 13)
#define HSP_VBUSVLDEXT				(0x1 << 12)
#define HSP_EN_UTMISUSPEND			(0x1 << 9)
#define HSP_COMMONONN				(0x1 << 8)
#define HSP_VATESTENB				(0x1 << 6)
#define HSP_CHGDET				(0x1 << 5)
#define HSP_CHGDET_GET(_x)		((_x & (0x1 << 5)) >> 5)
#define HSP_VDATSRCENB				(0x1 << 4)
#define HSP_VDATDETENB				(0x1 << 3)
#define HSP_CHRGSEL				(0x1 << 2)
#define HSP_ACAENB				(0x1 << 1)
#define HSP_DCDENB				(0x1 << 0)

#define EXYNOS_USBCON_HSP_TUNE		(0x58)
#define HSP_TUNE_TXVREF_MASK			((unsigned) 0xf << 28)
#define HSP_TUNE_TXVREF_SET(_x)			((unsigned) (_x & 0xf) << 28)
#define HSP_TUNE_TXVREF_GET(_x)			((_x & (0xfU << 28)) >> 28)
#define HSP_TUNE_TXRISE_MASK			(0x3 << 24)
#define HSP_TUNE_TXRISE_SET(_x)			((_x & 0x3) << 24)
#define HSP_TUNE_TXRISE_GET(_x)			((_x & (0x3 << 24)) >> 24)
#define HSP_TUNE_TXRES_MASK			(0x3 << 21)
#define HSP_TUNE_TXRES_SET(_x)			((_x & 0x3) << 21)
#define HSP_TUNE_TXRES_GET(_x)			((_x & (0x3 << 21)) >> 21)
#define HSP_TUNE_TXPREEMPA_PLUS			(0x1 << 20)
#define HSP_TUNE_TXPREEMPA_PLUS_GET(_x)	((_x & (0x1 << 20)) >> 20)
#define HSP_TUNE_TXPREEMPA_MASK			(0x3 << 18)
#define HSP_TUNE_TXPREEMPA_SET(_x)		((_x & 0x3) << 18)
#define HSP_TUNE_TXPREEMPA_GET(_x)		((_x & (0x3 << 18)) >> 18)
#define HSP_TUNE_HSXV_MASK			(0x3 << 16)
#define HSP_TUNE_HSXV_SET(_x)			((_x & 0x3) << 16)
#define HSP_TUNE_HSXV_GET(_x)			((_x & (0x3 << 16)) >> 16)
#define HSP_TUNE_TXFSLS_MASK			(0xf << 12)
#define HSP_TUNE_TXFSLS_SET(_x)			((_x & 0xf) << 12)
#define HSP_TUNE_TXFSLS_GET(_x)			((_x & (0xf << 12)) >> 12)
#define HSP_TUNE_SQRX_MASK			(0x7 << 8)
#define HSP_TUNE_SQRX_SET(_x)			((_x & 0x7) << 8)
#define HSP_TUNE_SQRX_GET(_x)			((_x & (0x7 << 8)) >> 8)
#define HSP_TUNE_OTG_MASK			(0x7 << 4)
#define HSP_TUNE_OTG_SET(_x)			((_x & 0x7) << 4)
#define HSP_TUNE_OTG_GET(_x)			((_x & (0x7 << 4)) >> 4)
#define HSP_TUNE_COMPDIS_MASK			(0x7 << 0)
#define HSP_TUNE_COMPDIS_SET(_x)		((_x & 0x7) << 0)
#define HSP_TUNE_COMPDIS_GET(_x)		((_x & (0x7 << 0)) >> 0)

#define EXYNOS_USBCON_HSP_TEST		(0x5c)
#define HSP_TEST_PHYCLKCNT_GET(_x)	((_x & (0x3f << 26)) >> 26)
#define HSP_TEST_HS_RXDAT			(0x1 << 26)
#define HSP_TEST_HS_SQUELCH			(0x1 << 25)
#define HSP_TEST_SIDDQ				(0x1 << 24)
#define HSP_TEST_LINESTATE_MASK			(0x3 << 20)
#define HSP_TEST_LINESTATE_SET(_x)		((_x & 0x3) << 20)
#define HSP_TEST_LINESTATE_GET(_x)		((_x & (0x3 << 20)) >> 20)
#define HSP_TEST_DATA_OUT_MASK			(0xf << 16)
#define HSP_TEST_DATA_OUT_SET(_x)		((_x & 0xf) << 16)
#define HSP_TEST_DATA_OUT_GET(_x)		((_x & (0xf << 16)) >> 16)
#define HSP_TEST_CLK				(0x1 << 13)
#define HSP_TEST_DATA_OUT_SEL			(0x1 << 12)
#define HSP_TEST_DATA_ADDR_MASK			(0xf << 8)
#define HSP_TEST_DATA_ADDR_SET(_x)		((_x & 0xf) << 8)
#define HSP_TEST_DATA_ADDR_GET(_x)		((_x & (0xf << 8)) >> 8)
#define HSP_TEST_DATA_IN_MASK			(0xff << 0)
#define HSP_TEST_DATA_IN_SET(_x)		((_x & 0xff) << 0)
#define HSP_TEST_DATA_IN_GET(_x)		((_x & (0xff << 0)) >> 0)

#define EXYNOS_USBCON_HSP_PLL_TUNE	(0x60)
#define HSP_PLL_BTUNE				(0x1 << 8)
#define HSP_PLL_ITUNE_MASK			(0x3 << 4)
#define HSP_PLL_ITUNE_IN_SET(_x)		((_x & 0x3) << 4)
#define HSP_PLL_ITUNE_IN_GET(_x)		((_x & (0x3 << 4)) >> 4)
#define HSP_PLL_PTUNE_MASK			(0xf << 0)
#define HSP_PLL_PTUNE_IN_SET(_x)		((_x & 0xf) << 0)
#define HSP_PLL_PTUNE_IN_GET(_x)		((_x & (0xf << 0)) >> 0)

#define EXYNOS_USBCON_G2PHY_CRPARCON0	(0x80)
#define G2PHY_CRPARCON0_CR_PARA_ADDR(_x)		((_x & 0xffff) << 16)
#define G2PHY_CRPARCON0_CR_PARA_ACK			(0x1 << 8)
#define G2PHY_CRPARCON0_CR_PARA_ACK_GET(_x)	((_x & (0x1 << 8)) >> 8)
#define G2PHY_CRPARCON0_CR_PARA_SEL			(0x1 << 4)
#define G2PHY_CRPARCON0_CR_PARA_CLK			(0x1 << 0)

#define EXYNOS_USBCON_G2PHY_CRPARCON1	(0x84)
#define G2PHY_CRPARCON1_CR_PARA_RD_DATA(_x)	((_x & 0xffff) << 16)
#define G2PHY_CRPARCON1_CR_PARA_RD_EN			(0x1 << 0)

#define EXYNOS_USBCON_G2PHY_CRPARCON2	(0x88)
#define G2PHY_CRPARCON2_CR_PARA_WR_DATA(_x)	((_x & 0xffff) << 16)
#define G2PHY_CRPARCON2_CR_PARA_WR_EN			(0x1 << 0)

#define EXYNOS_USBCON_G2PHY_CNTL0		(0x8c)
#define G2PHY_CNTL0_UPCS_PIPE_CONFIG(_x)		((_x & 0xffff) << 16)
#define G2PHY_CNTL0_DTB_OUT(_x)				((_x & 0x3) << 14)
#define G2PHY_CNTL0_UPCS_PWR_EN				(0x1 << 13)
#define G2PHY_CNTL0_UPCS_PWR_STABLE			(0x1 << 12)
#define G2PHY_CNTL0_SRAM_INIT_DONE			(0x1 << 10)
#define G2PHY_CNTL0_SRAM_EXT_LD_DONE			(0x1 << 9)
#define G2PHY_CNTL0_SRAM_BYPASS				(0x1 << 8)
#define G2PHY_CNTL0_EXT_PCLK_REQ				(0x1 << 7)
#define G2PHY_CNTL0_FORCE_PIPE_RX0_STANDBY	(0x1 << 6)
#define G2PHY_CNTL0_FORCE_PIPE_RX0_SRIS_MODE_EN	(0x1 << 5)
#define G2PHY_CNTL0_TEST_POWERDOWN			(0x1 << 4)
#define G2PHY_CNTL0_EXT_CTRL_SEL				(0x1 << 3)
#define G2PHY_CNTL0_FORCE_PHY_MODE			(0x1 << 2)
#define G2PHY_CNTL0_PHY_MODE(_x)				((_x & 0x3) << 0)

#define EXYNOS_USBCON_G2PHY_CNTL1		(0x90)
#define G2PHY_CNTL1_RX_RECAL_CONT_EN			(0x1 << 9)
#define G2PHY_CNTL1_RX2TX_PAR_LB_EN			(0x1 << 8)
#define G2PHY_CNTL1_MPLLB_SSC_EN				(0x1 << 5)
#define G2PHY_CNTL1_MPLLA_SSC_EN				(0x1 << 4)
#define G2PHY_CNTL1_PMA_PWR_STABLE			(0x1 << 2)
#define G2PHY_CNTL1_PCS_PWR_STABLE			(0x1 << 1)
#define G2PHY_CNTL1_ANA_PWR_EN				(0x1 << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT0	(0x94)
#define G2PHY_PRTCL1EXT0_BS_RX_BIGSWING		(0x1 << 12)
#define G2PHY_PRTCL1EXT0_BS_RX_LEVEL(_x)		((_x & 0x1f) << 4)
#define G2PHY_PRTCL1EXT0_BS_RX_LOWSWING		(0x1 << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT1	(0x98)
#define G2PHY_PRTCL1EXT1_MPLLA_BANDWIDTH(_x)	((_x & 0xffff) << 16)
#define G2PHY_PRTCL1EXT1_MPLLA_DIV10_CLK_EN		(0x1 << 11)
#define G2PHY_PRTCL1EXT1_MPLLA_DIV16P5_CLK_EN	(0x1 << 10)
#define G2PHY_PRTCL1EXT1_MPLLA_DIV8_CLK_EN		(0x1 << 9)
#define G2PHY_PRTCL1EXT1_MPLLA_DIV_CLK_EN		(0x1 << 8)
#define G2PHY_PRTCL1EXT1_MPLLA_DIV_MULTIPLIER(_x) ((_x & 0xff) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT2	(0x9c)
#define G2PHY_PRTCL1EXT2_MPLLA_FRACN_CTRL(_x)	((_x & 0x7ff) << 16)
#define G2PHY_PRTCL1EXT2_MPLLA_MULTIPLIER(_x)	((_x & 0xff) << 8)
#define G2PHY_PRTCL1EXT2_MPLLA_SSC_CLK_SEL(_x)	((_x & 0x3) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT3	(0xa0)
#define G2PHY_PRTCL1EXT3_MPLLA_SSC_FREQ_CNT_INIT(_x)	((_x & 0xfff) << 16)
#define G2PHY_PRTCL1EXT3_MPLLA_SSC_FREQ_CNT_PEAK(_x)	((_x & 0xff) << 8)
#define G2PHY_PRTCL1EXT3_MPLLA_SSC_FREQ_CNT_OVRD_EN		(0x1 << 7)
#define G2PHY_PRTCL1EXT3_MPLLA_SSC_RANGE(_x)			((_x & 0x7) << 4)
#define G2PHY_PRTCL1EXT3_MPLLA_TX_CLK_DIV(_x)			((_x & 0x7) << 1)
#define G2PHY_PRTCL1EXT3_MPLLA_WORD_DIV2_EN				(0x1 << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT4	(0xa4)
#define G2PHY_PRTCL1EXT4_MPLLB_BANDWIDTH(_x)			((_x & 0xffff) << 16)
#define G2PHY_PRTCL1EXT4_MPLLB_DIV10_CLK_EN				(0x1 << 11)
#define G2PHY_PRTCL1EXT4_MPLLB_DIV8_CLK_EN				(0x1 << 9)
#define G2PHY_PRTCL1EXT4_MPLLB_DIV_CLK_EN				(0x1 << 8)
#define G2PHY_PRTCL1EXT4_MPLLB_DIV_MULTIPLIER(_x)		((_x & 0xff) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT5	(0xa8)
#define G2PHY_PRTCL1EXT5_MPLLB_FRACN_CTRL(_x)			((_x & 0x7ff) << 16)
#define G2PHY_PRTCL1EXT5_MPLLB_MULTIPLIER(_x)			((_x & 0xff) << 8)
#define G2PHY_PRTCL1EXT5_MPLLB_SSC_CLK_SEL(_x)			((_x & 0x7) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT6	(0xac)
#define G2PHY_PRTCL1EXT6_MPLLB_SSC_FREQ_CNT_INIT(_x)	((_x & 0xfff) << 16)
#define G2PHY_PRTCL1EXT6_MPLLB_SSC_FREQ_CNT_PEAK(_x)	((_x & 0xff) << 8)
#define G2PHY_PRTCL1EXT6_MPLLB_SSC_FREQ_CNT_OVRD_EN		(0x1 << 7)
#define G2PHY_PRTCL1EXT6_MPLLB_SSC_RANGE(_x)			((_x & 0x7) << 4)
#define G2PHY_PRTCL1EXT6_MPLLB_TX_CLK_DIV(_x)			((_x & 0x7) << 1)
#define G2PHY_PRTCL1EXT6_MPLL_WORD_DIV2_EN				(0x1 << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT7	(0xb0)
#define G2PHY_PRTCL1EXT7_REF_CLK_DIV2_EN				(0x1 << 29)
#define G2PHY_PRTCL1EXT7_REF_CLK_MPLLA_DIV2_EN			(0x1 << 28)
#define G2PHY_PRTCL1EXT7_REF_CLK_MPLLB_DIV2_EN			(0x1 << 27)
#define G2PHY_PRTCL1EXT7_REF_RANGE(_x)					((_x & 0x7) << 24)
#define G2PHY_PRTCL1EXT7_RX_ADAPT_AFE_EN_G2				(0x1 << 21)
#define G2PHY_PRTCL1EXT7_RX_ADAPT_AFE_EN_G1				(0x1 << 20)
#define G2PHY_PRTCL1EXT7_RX_ADAPT_DFE_EN_G2				(0x1 << 17)
#define G2PHY_PRTCL1EXT7_RX_ADAPT_DFE_EN_G1				(0x1 << 16)
#define G2PHY_PRTCL1EXT7_RX_EQ_AFE_GAIN_G2(_x)			((_x & 0xf) << 4)
#define G2PHY_PRTCL1EXT7_RX_EQ_AFE_GAIN_G1(_x)			((_x & 0xf) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT8	(0xb4)
#define G2PHY_PRTCL1EXT8_RX_EQ_DELTA_IQ_G2(_x)			((_x & 0xf) << 20)
#define G2PHY_PRTCL1EXT8_RX_EQ_DELTA_IQ_G1(_x)			((_x & 0xf) << 16)
#define G2PHY_PRTCL1EXT8_RX_EQ_ATT_LVL_G2(_x)			((_x & 0x7) << 4)
#define G2PHY_PRTCL1EXT8_RX_EQ_ATT_LVL_G1(_x)			((_x & 0x7) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT9	(0xb8)
#define G2PHY_PRTCL1EXT9_RX_EQ_CTLE_BOOT_G2(_x)			((_x & 0x1f) << 8)
#define G2PHY_PRTCL1EXT9_RX_EQ_CTLE_BOOT_G1(_x)			((_x & 0x1f) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT10	(0xbc)
#define G2PHY_PRTCL1EXT10_RX_EQ_DFE_TAP1_G2(_x)			((_x & 0xff) << 8)
#define G2PHY_PRTCL1EXT10_RX_EQ_DFE_TAP1_G1(_x)			((_x & 0xff) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT11	(0xc0)
#define G2PHY_PRTCL1EXT11_RX_VREF_CTRL(_x)				((_x & 0x1f) << 24)
#define G2PHY_PRTCL1EXT11_RX_LOS_LFPS_EN				(0x1 << 23)
#define G2PHY_PRTCL1EXT11_RX_LOS_THRESHOLD(_x)			((_x & 0x7) << 20)
#define G2PHY_PRTCL1EXT11_RX_TERM_CTRL(_x)				((_x & 0x7) << 16)
#define G2PHY_PRTCL1EXT11_RX_REF_LD_VAL_G2(_x)			((_x & 0x3f) << 8)
#define G2PHY_PRTCL1EXT11_RX_REF_LD_VAL_G1(_x)			((_x & 0x3f) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT12	(0xc4)
#define G2PHY_PRTCL1EXT12_RX_LOS_PWR_UP_CNT				((_x & 0x7ff) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT13	(0xc8)
#define G2PHY_PRTCL1EXT13_RX_VCO_LD_VAL_G2(_x)			((_x & 0x1fff) << 16)
#define G2PHY_PRTCL1EXT13_RX_VCO_LD_VAL_G1(_x)			((_x & 0x1fff) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT14	(0xcc)
#define G2PHY_PRTCL1EXT14_TX_EQ_OVRD_G2					(0x1 << 17)
#define G2PHY_PRTCL1EXT14_TX_EQ_OVRD_G1					(0x1 << 16)
#define G2PHY_PRTCL1EXT14_TX_EQ_POST_G2(_x)				((_x & 0xf) << 12)
#define G2PHY_PRTCL1EXT14_TX_EQ_POST_G1(_x)				((_x & 0xf) << 8)
#define G2PHY_PRTCL1EXT14_TX_EQ_PRE_G2(_x)				((_x & 0xf) << 4)
#define G2PHY_PRTCL1EXT14_TX_EQ_PRE_G1(_x)				((_x & 0xf) << 0)

#define EXYNOS_USBCON_G2PHY_PRTCL1EXT15	(0xd0)
#define G2PHY_PRTCL1EXT15_TX_IBOOST_LVL_MASK			(0xf << 24)
#define G2PHY_PRTCL1EXT15_TX_IBOOST_LVL(_x)				((_x & 0xf) << 24)
#define G2PHY_PRTCL1EXT15_TX_TERM_CTRL_MASK				(0x7 << 20)
#define G2PHY_PRTCL1EXT15_TX_TERM_CTRL(_x)				((_x & 0x7) << 20)
#define G2PHY_PRTCL1EXT15_TX_VBOOST_LVL_MASK			(0x7 << 16)
#define G2PHY_PRTCL1EXT15_TX_VBOOST_LVL(_x)				((_x & 0x7) << 16)
#define G2PHY_PRTCL1EXT15_TX_EQ_MAIN_G2_MASK			(0x1f << 8)
#define G2PHY_PRTCL1EXT15_TX_EQ_MAIN_G2(_x)				((_x & 0x1f) << 8)
#define G2PHY_PRTCL1EXT15_TX_EQ_MAIN_G1_MASK			(0x1f << 0)
#define G2PHY_PRTCL1EXT15_TX_EQ_MAIN_G1(_x)				((_x & 0x1f) << 0)
#define EXYNOS_USBCON_ESS_CTL		(0x70)
#define ESS_DISRXDETU3RXDET			(0x1 << 9)

/* ---- CAL ----------------------------------------------------------------- */

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
 * @regs_base: base address of PHY control register *
 */

struct exynos_usbphy_info {
	/* Device Information */
	struct device *dev;

	u32 version;
	enum exynos_usbphy_refclk refclk;

	void __iomem *regs_base;

	/* multiple phy */
	int hw_version;
	void __iomem *pma_base;
	void __iomem *pcs_base;
	void __iomem *ctrl_base;
	void __iomem *link_base;
};

/* -------------------------------------------------------------------------- */

#define EXYNOS_USBPHY_VER_02_0_0	0x0200	/* Lhotse - USBDP Combo PHY */

/* 9810 PMU register offset */
#define EXYNOS_USBDP_PHY_CONTROL	(0x704)
#define EXYNOS_USB2_PHY_CONTROL	(0x72C)
/* PMU register offset for USB */
#define EXYNOS_USBDEV_PHY_CONTROL	(0x704)
#define EXYNOS_USBDRD_ENABLE		BIT(0)
#define EXYNOS_USBHOST_ENABLE		BIT(1)

/* Exynos USB PHY registers */
#define EXYNOS_FSEL_9MHZ6		0x0
#define EXYNOS_FSEL_10MHZ		0x1
#define EXYNOS_FSEL_12MHZ		0x2
#define EXYNOS_FSEL_19MHZ2		0x1
#define EXYNOS_FSEL_20MHZ		0x4
#define EXYNOS_FSEL_24MHZ		0x5
#define EXYNOS_FSEL_26MHZ		0x82
#define EXYNOS_FSEL_50MHZ		0x7

/* EXYNOS: USB DRD PHY registers */
#define EXYNOS_DRD_LINKSYSTEM			0x04

#define LINKSYSTEM_FLADJ_MASK			(0x3f << 1)
#define LINKSYSTEM_FLADJ(_x)			((_x) << 1)

#define EXYNOS_DRD_PHYUTMI			0x08

#define EXYNOS_DRD_PHYPIPE			0x0c

#define PHYPIPE_PHY_CLOCK_SEL				(0x1 << 4)

#define EXYNOS_DRD_PHYCLKRST			0x10

#define PHYCLKRST_SSC_REFCLKSEL_MASK		(0xff << 23)
#define PHYCLKRST_SSC_REFCLKSEL(_x)		((_x) << 23)

#define PHYCLKRST_SSC_RANGE_MASK		(0x03 << 21)
#define PHYCLKRST_SSC_RANGE(_x)			((_x) << 21)

#define PHYCLKRST_MPLL_MULTIPLIER_MASK		(0x7f << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_100MHZ_REF	(0x19 << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_50M_REF	(0x32 << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_24MHZ_REF	(0x68 << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_20MHZ_REF	(0x7d << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_19200KHZ_REF	(0x02 << 11)

#define PHYCLKRST_FSEL_UTMI_MASK		(0x7 << 5)
#define PHYCLKRST_FSEL_PIPE_MASK		(0x7 << 8)
#define PHYCLKRST_FSEL(_x)			((_x) << 5)
#define PHYCLKRST_FSEL_PAD_100MHZ		(0x27 << 5)
#define PHYCLKRST_FSEL_PAD_24MHZ		(0x2a << 5)
#define PHYCLKRST_FSEL_PAD_20MHZ		(0x31 << 5)
#define PHYCLKRST_FSEL_PAD_19_2MHZ		(0x38 << 5)

#define PHYCLKRST_REFCLKSEL_MASK		(0x03 << 2)
#define PHYCLKRST_REFCLKSEL_PAD_REFCLK		(0x2 << 2)
#define PHYCLKRST_REFCLKSEL_EXT_REFCLK		(0x3 << 2)

#define EXYNOS_DRD_PHYREG0			0x14
#define EXYNOS_DRD_PHYREG1			0x18

#define EXYNOS_DRD_PHYPARAM0			0x1c

#define PHYPARAM0_REF_LOSLEVEL_MASK		(0x1f << 26)
#define PHYPARAM0_REF_LOSLEVEL			(0x9 << 26)

#define EXYNOS_DRD_PHYPARAM1			0x20

#define PHYPARAM1_PCS_TXDEEMPH_MASK		(0x1f << 0)
#define PHYPARAM1_PCS_TXDEEMPH			(0x1c)

#define EXYNOS_DRD_PHYTERM			0x24

#define EXYNOS_DRD_PHYTEST			0x28

#define EXYNOS_DRD_PHYADP			0x2c

#define EXYNOS_DRD_PHYUTMICLKSEL		0x30

#define PHYUTMICLKSEL_UTMI_CLKSEL		BIT(2)

#define EXYNOS_DRD_PHYRESUME			0x34
#define EXYNOS_DRD_LINKPORT			0x44

#define KHZ	1000
#define MHZ	(KHZ * KHZ)

#define EXYNOS_DRD_MAX_TUNEPARAM_NUM		32

enum exynos_usbdrd_phy_id {
	EXYNOS_DRDPHY_UTMI,
	EXYNOS_DRDPHYS_NUM,
};

struct phy_usb_instance;
struct exynos_usbdrd_phy;

struct exynos_usbdrd_phy_config {
	u32 id;
	void (*phy_isol)(struct phy_usb_instance *inst, u32 on, unsigned int);
	void (*phy_init)(struct exynos_usbdrd_phy *phy_drd);
	void (*phy_exit)(struct exynos_usbdrd_phy *phy_drd);
	unsigned int (*set_refclk)(struct phy_usb_instance *inst);
};

struct exynos_usbdrd_phy_drvdata {
	const struct exynos_usbdrd_phy_config *phy_cfg;
};

/**
 * struct exynos_usbdrd_phy - driver data for USB DRD PHY
 * @dev: pointer to device instance of this platform device
 * @reg_phy: usb phy controller register memory base
 * @clk: phy clock for register access
 * @drv_data: pointer to SoC level driver data structure
 * @phys[]: array for 'EXYNOS_DRDPHYS_NUM' number of PHY
 *	    instances each with its 'phy' and 'phy_cfg'.
 * @extrefclk: frequency select settings when using 'separate
 *	       reference clocks' for HS operations
 * @ref_clk: reference clock to PHY block from which PHY's
 *	     operational clocks are derived
 * @usbphy_info; Phy main control info
 */
struct exynos_usbdrd_phy {
	struct device *dev;
	void __iomem *reg_phy;
	void __iomem *reg_phy2;
	void __iomem *reg_phy3;
	void __iomem *reg_link;
	void __iomem *reg_eusb_ctrl;
	void __iomem *reg_eusb_phy;
	void __iomem *reg_dpphy_ctrl;
	void __iomem *reg_dpphy_tca;
	struct clk **clocks;
	const struct exynos_usbdrd_phy_drvdata *drv_data;
	struct phy_usb_instance {
		struct phy *phy;
		u32 index;
		struct regmap *reg_pmu;
		u32 pmu_offset;
		u32 pmu_offset_dp;
		u32 pmu_mask;
		const struct exynos_usbdrd_phy_config *phy_cfg;
	} phys[EXYNOS_DRDPHYS_NUM];
	u32 extrefclk;
	struct clk *ref_clk;
	struct regulator *vbus;
	struct regulator	*vdd075_usb;
	struct regulator	*vdd12_usb;
	struct regulator	*vdd18_usb;
	struct regulator	*vdd33_usb;
	struct exynos_usbphy_info usbphy_info;
	struct exynos_usbphy_info usbphy_blkcon_info;

	int is_conn;
	int idle_ip_idx;
	spinlock_t lock;
	int in_shutdown;
	int is_ldo_on;
};

/* -------------------------------------------------------------------------- */

static void exynos_cal_usbphy_q_ch(void *regs_base, u8 enable)
{
	u32 phy_resume;

	if (enable) {
		/* WA for Q-channel: disable all q-act from usb */
		phy_resume = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
		phy_resume |= LINKCTRL_DIS_QACT_ID0;
		phy_resume |= LINKCTRL_DIS_QACT_VBUS_VALID;
		phy_resume |= LINKCTRL_DIS_QACT_BVALID;
		phy_resume |= LINKCTRL_DIS_QACT_LINKGATE;
		phy_resume &= ~LINKCTRL_FORCE_QACT;
		udelay(500);
		writel(phy_resume, regs_base + EXYNOS_USBCON_LINK_CTRL);
		udelay(500);
		phy_resume = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
		phy_resume |= LINKCTRL_FORCE_QACT;
		udelay(500);
		writel(phy_resume, regs_base + EXYNOS_USBCON_LINK_CTRL);
	} else {
		phy_resume = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
		phy_resume &= ~LINKCTRL_FORCE_QACT;
		phy_resume |= LINKCTRL_DIS_QACT_ID0;
		phy_resume |= LINKCTRL_DIS_QACT_VBUS_VALID;
		phy_resume |= LINKCTRL_DIS_QACT_BVALID;
		phy_resume |= LINKCTRL_DIS_QACT_LINKGATE;
		writel(phy_resume, regs_base + EXYNOS_USBCON_LINK_CTRL);
	}
}

static void link_vbus_filter_en(struct exynos_usbphy_info *info,
	u8 enable)
{
	u32 phy_resume;

	phy_resume = readl(info->regs_base + EXYNOS_USBCON_LINK_CTRL);
	if (enable)
		phy_resume &= ~LINKCTRL_BUS_FILTER_BYPASS_MASK;
	else
		phy_resume |= LINKCTRL_BUS_FILTER_BYPASS(0xf);
	writel(phy_resume, info->regs_base + EXYNOS_USBCON_LINK_CTRL);
}

static void phy_power_en(struct exynos_usbphy_info *info, u8 en)
{
	u32 reg;
	int main_version;

	main_version = info->version & EXYNOS_USBCON_VER_MAJOR_VER_MASK;

	if (main_version == EXYNOS_USBCON_VER_03_0_0) {
		/* 2.0 PHY Power Down Control */
		reg = readl(info->regs_base + EXYNOS_USBCON_HSP_TEST);
		if (en)
			reg &= ~HSP_TEST_SIDDQ;
		else
			reg |= HSP_TEST_SIDDQ;
		writel(reg, info->regs_base + EXYNOS_USBCON_HSP_TEST);
	}
}

static void phy_sw_rst_high(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 clkrst;

	clkrst = readl(regs_base + EXYNOS_USBCON_CLKRST);
	if (EXYNOS_USBCON_VER_MINOR(info->version) >= 0x1) {
		clkrst |= CLKRST_PHY20_SW_RST;
		clkrst |= CLKRST_PHY20_RST_SEL;
		clkrst |= CLKRST_PHY30_SW_RST;
		clkrst |= CLKRST_PHY30_RST_SEL;
	} else {
		clkrst |= CLKRST_PHY_SW_RST;
		clkrst |= CLKRST_PHY_RST_SEL;
		clkrst |= CLKRST_PORT_RST;
	}
	writel(clkrst, regs_base + EXYNOS_USBCON_CLKRST);
}

static void phy_sw_rst_low(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 clkrst;

	clkrst = readl(regs_base + EXYNOS_USBCON_CLKRST);
	if (EXYNOS_USBCON_VER_MINOR(info->version) >= 0x1) {
		clkrst |= CLKRST_PHY20_RST_SEL;
		clkrst &= ~CLKRST_PHY20_SW_RST;
		clkrst &= ~CLKRST_PHY30_SW_RST;
		clkrst &= ~CLKRST_PORT_RST;
	} else {
		clkrst |= CLKRST_PHY_RST_SEL;
		clkrst &= ~CLKRST_PHY_SW_RST;
		clkrst &= ~CLKRST_PORT_RST;
	}
	writel(clkrst, regs_base + EXYNOS_USBCON_CLKRST);
}

/* USB/DP PHY control */
static void phy_exynos_usb_v3p1_pipe_ovrd(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;

	/* force pipe3 signal for link */
	reg = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
	reg |= LINKCTRL_PIPE3_FORCE_EN;
	reg &= ~LINKCTRL_PIPE3_FORCE_PHY_STATUS;
	reg |= LINKCTRL_PIPE3_FORCE_RX_ELEC_IDLE;
	writel(reg, regs_base + EXYNOS_USBCON_LINK_CTRL);

	/* PMA Disable */
	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg |= PMA_LOW_PWR;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
}

static void phy_exynos_usb_v3p1_link_sw_reset(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	int main_version;
	u32 reg;

	main_version = info->version & EXYNOS_USBCON_VER_MAJOR_VER_MASK;

	/*
	 * Use link_sw_rst because it has functioning as Hreset_n
	 * for asb host/device role change, originally not recommend link_sw_rst
	 * by Foundry T. so that some of global register has cleard - 2018.11.12
	 */
	/* Link Reset */
	if (main_version == EXYNOS_USBCON_VER_03_0_0) {
		reg = readl(info->regs_base + EXYNOS_USBCON_CLKRST);
		reg |= CLKRST_LINK_SW_RST;
		writel(reg, regs_base + EXYNOS_USBCON_CLKRST);

		udelay(10);

		reg &= ~CLKRST_LINK_SW_RST;
		writel(reg, regs_base + EXYNOS_USBCON_CLKRST);
	}
}

static void phy_exynos_usb_v3p1_enable(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;
	u32 reg_hsp;
	int main_version;

	main_version = info->version & EXYNOS_USBCON_VER_MAJOR_VER_MASK;

	if (main_version == EXYNOS_USBCON_VER_03_0_0) {
		/* Set force q-channel */
		exynos_cal_usbphy_q_ch(regs_base, 1);
	}

	/* Set PHY POR High */
	phy_sw_rst_high(info);

	reg = readl(regs_base + EXYNOS_USBCON_UTMI);
	reg &= ~UTMI_FORCE_SUSPEND;
	reg &= ~UTMI_FORCE_SLEEP;
	reg &= ~UTMI_DP_PULLDOWN;
	reg &= ~UTMI_DM_PULLDOWN;
	writel(reg, regs_base + EXYNOS_USBCON_UTMI);

	/* set phy clock & control HS phy */
	reg = readl(regs_base + EXYNOS_USBCON_HSP);
	reg |= HSP_EN_UTMISUSPEND;
	reg |= HSP_COMMONONN;
	writel(reg, regs_base + EXYNOS_USBCON_HSP);

	udelay(100);

	/* Follow setting sequence for USB Link */
	/* 1. Set VBUS Valid and DP-Pull up control
	 * by VBUS pad usage
	 */
	link_vbus_filter_en(info, false);
	reg = readl(regs_base + EXYNOS_USBCON_UTMI);
	reg_hsp = readl(regs_base + EXYNOS_USBCON_HSP);
	reg |= UTMI_FORCE_BVALID;
	reg |= UTMI_FORCE_VBUSVALID;
	reg_hsp |= HSP_VBUSVLDEXTSEL;
	reg_hsp |= HSP_VBUSVLDEXT;

	writel(reg, regs_base + EXYNOS_USBCON_UTMI);
	writel(reg_hsp, regs_base + EXYNOS_USBCON_HSP);

	/* Enable PHY Power Mode */
	phy_power_en(info, 1);

	/* before POR low, 10us delay is needed. */
	udelay(10);

	/* Set PHY POR Low */
	phy_sw_rst_low(info);

	/* after POR low and delay 75us, PHYCLOCK is guaranteed. */
	udelay(75);

	/* 2. OVC io usage */
	reg = readl(regs_base + EXYNOS_USBCON_LINK_PORT);
	reg |= LINKPORT_HUB_PORT_SEL_OCD_U3;
	reg |= LINKPORT_HUB_PORT_SEL_OCD_U2;
	writel(reg, regs_base + EXYNOS_USBCON_LINK_PORT);
}

static void phy_exynos_usb_v3p1_disable(struct exynos_usbphy_info *info)
{
	u32 reg;
	void __iomem *regs_base = info->regs_base;

	/* set phy clock & control HS phy */
	reg = readl(regs_base + EXYNOS_USBCON_UTMI);
	reg &= ~UTMI_DP_PULLDOWN;
	reg &= ~UTMI_DM_PULLDOWN;
	reg |= UTMI_FORCE_SUSPEND;
	reg |= UTMI_FORCE_SLEEP;
	writel(reg, regs_base + EXYNOS_USBCON_UTMI);

	/* Disable PHY Power Mode */
	phy_power_en(info, 0);

	/* clear force q-channel */
	exynos_cal_usbphy_q_ch(regs_base, 0);

	/*
	 * Link sw reset is need for USB_DP/DM high-z in host mode: 2019.04.10
	 * by daeman.ko.
	 */
	phy_exynos_usb_v3p1_link_sw_reset(info);
}

/* UART/JTAG over USB */
static void phy_exynos_usb3p1_set_fsv_out_en(
		struct exynos_usbphy_info *usbphy_info, u32 fsv_out_en)
{
	void __iomem *regs_base = usbphy_info->regs_base;
	u32 hsp_ctrl;

	hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
	if (fsv_out_en) {
		hsp_ctrl |= HSP_FSVP_OUT_EN;
		hsp_ctrl |= HSP_FSVM_OUT_EN;
	} else {
		hsp_ctrl &= ~HSP_FSVP_OUT_EN;
		hsp_ctrl &= ~HSP_FSVM_OUT_EN;
	}
	writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);
}

/* -------------------------------------------------------------------------- */

static int exynos_usbdrd_clk_prepare(struct exynos_usbdrd_phy *phy_drd)
{
	int i;
	int ret;

	for (i = 0; phy_drd->clocks[i] != NULL; i++) {
		ret = clk_prepare(phy_drd->clocks[i]);
		if (ret)
			goto err;
	}

	return 0;

err:
	for (i = i - 1; i >= 0; i--)
		clk_unprepare(phy_drd->clocks[i]);
	return ret;
}

static int exynos_usbdrd_clk_enable(struct exynos_usbdrd_phy *phy_drd)
{
	int i;
	int ret;

	for (i = 0; phy_drd->clocks[i] != NULL; i++) {
		ret = clk_enable(phy_drd->clocks[i]);
		if (ret)
			goto err;
	}
	return 0;

err:
	for (i = i - 1; i >= 0; i--)
		clk_disable(phy_drd->clocks[i]);
	return ret;
}

static void exynos_usbdrd_clk_unprepare(struct exynos_usbdrd_phy *phy_drd)
{
	int i;

	for (i = 0; phy_drd->clocks[i] != NULL; i++)
		clk_unprepare(phy_drd->clocks[i]);
}

static void exynos_usbdrd_clk_disable(struct exynos_usbdrd_phy *phy_drd)
{
	int i;

	for (i = 0; phy_drd->clocks[i] != NULL; i++)
		clk_disable(phy_drd->clocks[i]);
}
static int exynos_usbdrd_phyclk_get(struct exynos_usbdrd_phy *phy_drd)
{
	struct device *dev = phy_drd->dev;
	const char	**clk_ids;
	const char	*refclk_name;
	struct clk	*clk;
	int		clk_count;
	bool		is_phyclk = false;
	int		clk_index = 0;
	int		i, ret;

	clk_count = of_property_count_strings(dev->of_node, "clock-names");
	if (IS_ERR_VALUE((unsigned long)clk_count)) {
		dev_err(dev, "invalid clk list in %s node", dev->of_node->name);
		return -EINVAL;
	}
	clk_ids = (const char **)devm_kmalloc(dev,
				(clk_count + 1) * sizeof(const char *),
				GFP_KERNEL);
	for (i = 0; i < clk_count; i++) {
		ret = of_property_read_string_index(dev->of_node, "clock-names",
								i, &clk_ids[i]);
		if (ret) {
			dev_err(dev, "failed to read clocks name %d from %s node\n",
					i, dev->of_node->name);
			return ret;
		}
	}
	clk_ids[clk_count] = NULL;

	phy_drd->clocks = (struct clk **) devm_kmalloc(dev,
				(clk_count + 1) * sizeof(struct clk *), GFP_KERNEL);
	if (!phy_drd->clocks) {
		dev_err(dev, "failed to alloc for clocks\n");
		return -ENOMEM;
	}

	for (i = 0; clk_ids[i] != NULL; i++) {
		if (!is_phyclk) {
			clk = devm_clk_get(dev, clk_ids[i]);
			if (IS_ERR_OR_NULL(clk)) {
				dev_err(dev, "couldn't get %s clock\n", clk_ids[i]);
				return -EINVAL;
			}
			phy_drd->clocks[clk_index] = clk;
			clk_index++;
		}
		is_phyclk = false;
	}
	phy_drd->clocks[clk_index] = NULL;

	ret = of_property_read_string_index(dev->of_node,
						"phy_refclk", 0, &refclk_name);
	if (ret) {
		dev_err(dev, "failed to read ref_clocks name from %s node\n",
				dev->of_node->name);
		return ret;
	}

	if (!strcmp("none", refclk_name)) {
		dev_err(dev, "phy reference clock shouldn't be omitted");
		return -EINVAL;
	}

	for (i = 0; clk_ids[i] != NULL; i++) {
		if (!strcmp(clk_ids[i], refclk_name)) {
			phy_drd->ref_clk = devm_clk_get(dev, refclk_name);
			break;
		}
	}

	if (IS_ERR_OR_NULL(phy_drd->ref_clk)) {
		dev_err(dev, "%s couldn't get ref_clk", __func__);
		return -EINVAL;
	}

	devm_kfree(dev, clk_ids);

	return 0;

}

static int exynos_usbdrd_clk_get(struct exynos_usbdrd_phy *phy_drd)
{
	struct device *dev = phy_drd->dev;
	int		ret;

	ret = exynos_usbdrd_phyclk_get(phy_drd);
	if (ret < 0) {
		dev_err(dev, "failed to get clock for DRD USBPHY");
		return ret;
	}

	return 0;
}

static inline
struct exynos_usbdrd_phy *to_usbdrd_phy(struct phy_usb_instance *inst)
{
	return container_of((inst), struct exynos_usbdrd_phy,
			    phys[(inst)->index]);
}

/*
 * exynos_rate_to_clk() converts the supplied clock rate to the value that
 * can be written to the phy register.
 */
static unsigned int exynos_rate_to_clk(struct exynos_usbdrd_phy *phy_drd)
{
	int ret;

	ret = clk_prepare_enable(phy_drd->ref_clk);
	if (ret) {
		dev_err(phy_drd->dev, "%s failed to enable ref_clk", __func__);
		return 0;
	}

	/* EXYNOS_FSEL_MASK */
	switch (clk_get_rate(phy_drd->ref_clk)) {
	case 9600 * KHZ:
		phy_drd->extrefclk = EXYNOS_FSEL_9MHZ6;
		break;
	case 10 * MHZ:
		phy_drd->extrefclk = EXYNOS_FSEL_10MHZ;
		break;
	case 12 * MHZ:
		phy_drd->extrefclk = EXYNOS_FSEL_12MHZ;
		break;
	case 19200 * KHZ:
		phy_drd->extrefclk = EXYNOS_FSEL_19MHZ2;
		break;
	case 20 * MHZ:
		phy_drd->extrefclk = EXYNOS_FSEL_20MHZ;
		break;
	case 24 * MHZ:
		phy_drd->extrefclk = EXYNOS_FSEL_24MHZ;
		break;
	case 26 * MHZ:
		phy_drd->extrefclk = EXYNOS_FSEL_26MHZ;
		break;
	case 50 * MHZ:
		phy_drd->extrefclk = EXYNOS_FSEL_50MHZ;
		break;
	default:
		phy_drd->extrefclk = 0;
		clk_disable_unprepare(phy_drd->ref_clk);
		return -EINVAL;
	}

	clk_disable_unprepare(phy_drd->ref_clk);

	return 0;
}

static void exynos_usbdrd_utmi_phy_isol(struct phy_usb_instance *inst,
					unsigned int on, unsigned int mask)
{
	unsigned int val;

	if (!inst->reg_pmu)
		return;

	val = on ? 0 : mask;
	regmap_update_bits(inst->reg_pmu, inst->pmu_offset,
		mask, val);
}

/*
 * Sets the utmi phy's clk as EXTREFCLK (XXTI) which is internal clock
 * from clock core. Further sets the FSEL values for HighSpeed operations.
 */
static unsigned int
exynos_usbdrd_utmi_set_refclk(struct phy_usb_instance *inst)
{
	static u32 reg;
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	/* PHYCLKRST setting isn't required in Combo PHY */
	if(phy_drd->usbphy_info.version >= EXYNOS_USBPHY_VER_02_0_0)
		return EINVAL;

	/* restore any previous reference clock settings */
	reg = readl(phy_drd->reg_phy + EXYNOS_DRD_PHYCLKRST);

	reg &= ~PHYCLKRST_REFCLKSEL_MASK;
	reg |=	PHYCLKRST_REFCLKSEL_EXT_REFCLK;

	reg &= ~PHYCLKRST_FSEL_UTMI_MASK |
		PHYCLKRST_MPLL_MULTIPLIER_MASK |
		PHYCLKRST_SSC_REFCLKSEL_MASK;
	reg |= PHYCLKRST_FSEL(phy_drd->extrefclk);

	return reg;
}

static int exynos_usbdrd_get_phyinfo(struct exynos_usbdrd_phy *phy_drd)
{
	struct device *dev = phy_drd->dev;
	int value;

	if (!of_property_read_u32(dev->of_node, "phy_version", &value)) {
		phy_drd->usbphy_info.version = value;
	} else {
		dev_err(dev, "can't get phy_version\n");
		return -EINVAL;
	}

	phy_drd->usbphy_info.refclk = phy_drd->extrefclk;
	phy_drd->usbphy_info.regs_base = phy_drd->reg_phy;

	dev_info(phy_drd->dev, "usbphy info: version:0x%x, refclk:0x%x\n",
		phy_drd->usbphy_info.version, phy_drd->usbphy_info.refclk);

	return 0;
}

static void exynos_usbdrd_utmi_exit(struct exynos_usbdrd_phy *phy_drd)
{
	phy_exynos_usb_v3p1_disable(&phy_drd->usbphy_info);

	exynos_usbdrd_clk_disable(phy_drd);

	exynos_usbdrd_utmi_phy_isol(&phy_drd->phys[0], 1,
				    phy_drd->phys[0].pmu_mask);
}

static int exynos_usbdrd_phy_exit(struct phy *phy)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	/* UTMI or PIPE3 specific exit */
	inst->phy_cfg->phy_exit(phy_drd);

	return 0;
}

static void exynos_usbdrd_utmi_init(struct exynos_usbdrd_phy *phy_drd)
{
	int ret;

	pr_info("%s: +++\n", __func__);

	exynos_usbdrd_utmi_phy_isol(&phy_drd->phys[0], 0,
				    phy_drd->phys[0].pmu_mask);

	ret = exynos_usbdrd_clk_enable(phy_drd);
	if (ret) {
		dev_err(phy_drd->dev, "%s: Failed to enable clk\n", __func__);
		return;
	}

	phy_exynos_usb_v3p1_enable(&phy_drd->usbphy_info);
	phy_exynos_usb_v3p1_pipe_ovrd(&phy_drd->usbphy_info);

	phy_exynos_usb3p1_set_fsv_out_en(&phy_drd->usbphy_info, 0);

	pr_info("%s: ---\n", __func__);
}

static int exynos_usbdrd_phy_init(struct phy *phy)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	/* UTMI or PIPE3 specific init */
	inst->phy_cfg->phy_init(phy_drd);

	return 0;
}

void exynos_usbdrd_ldo_control(struct exynos_usbdrd_phy *phy_drd, int on)
{
	int ret1, ret2, ret3;

	if (IS_ERR(phy_drd->vdd075_usb) || IS_ERR(phy_drd->vdd18_usb) || IS_ERR(phy_drd->vdd33_usb) || 
			phy_drd->vdd075_usb == NULL || phy_drd->vdd18_usb == NULL || phy_drd->vdd33_usb == NULL) {
		dev_err(phy_drd->dev, "%s: not define regulator\n", __func__);
		goto out;
	}

	dev_info(phy_drd->dev, "Turn %s LDO\n", on ? "on" : "off");

	if (on) {
		if (phy_drd->is_ldo_on) {
			dev_info(phy_drd->dev, "LDO already on! return\n");
			goto out;
		}
		ret1 = regulator_enable(phy_drd->vdd075_usb);
		ret2 = regulator_enable(phy_drd->vdd18_usb);
		ret3 = regulator_enable(phy_drd->vdd33_usb);
		if (ret1 || ret2 || ret3) {
			dev_err(phy_drd->dev, "Failed to enable USB LDOs: %d %d %d\n",
					ret1, ret2, ret3);
		}
		phy_drd->is_ldo_on = 1;
	} else {
		if (!phy_drd->is_ldo_on) {
			dev_info(phy_drd->dev, "LDO already off! return\n");
			goto out;
		}
		ret1 = regulator_disable(phy_drd->vdd075_usb);
		ret2 = regulator_disable(phy_drd->vdd18_usb);
		ret3 = regulator_disable(phy_drd->vdd33_usb);
		if (ret1 || ret2 || ret3) {
			dev_err(phy_drd->dev, "Failed to disable USB LDOs: %d %d %d\n",
					ret1, ret2, ret3);
		}
		phy_drd->is_ldo_on = 0;
	}
out:
	return;
}

int exynos_usbdrd_phy_link_rst(struct phy *phy)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	pr_info("%s\n", __func__);
	phy_exynos_usb_v3p1_link_sw_reset(&phy_drd->usbphy_info);
	return 0;
}

static int exynos_usbdrd_phy_power_on(struct phy *phy)
{
	return 0;
}

static int exynos_usbdrd_phy_power_off(struct phy *phy)
{
	return 0;
}

static struct phy *exynos_usbdrd_phy_xlate(struct device *dev,
					struct of_phandle_args *args)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_drvdata(dev);

	if (WARN_ON(args->args[0] > EXYNOS_DRDPHYS_NUM))
		return ERR_PTR(-ENODEV);

	return phy_drd->phys[args->args[0]].phy;
}

static struct phy_ops exynos_usbdrd_phy_ops = {
	.init		= exynos_usbdrd_phy_init,
	.exit		= exynos_usbdrd_phy_exit,
	.power_on	= exynos_usbdrd_phy_power_on,
	.power_off	= exynos_usbdrd_phy_power_off,
	.reset		= exynos_usbdrd_phy_link_rst,
	.owner		= THIS_MODULE,
};

static const struct exynos_usbdrd_phy_config phy_cfg_exynos[] = {
	{
		.id		= EXYNOS_DRDPHY_UTMI,
		.phy_isol	= exynos_usbdrd_utmi_phy_isol,
		.phy_init	= exynos_usbdrd_utmi_init,
		.phy_exit	= exynos_usbdrd_utmi_exit,
		.set_refclk	= exynos_usbdrd_utmi_set_refclk,
	},
};

static const struct exynos_usbdrd_phy_drvdata exynos_usbdrd_phy = {
	.phy_cfg		= phy_cfg_exynos,
};

static const struct of_device_id exynos_usbdrd_phy_of_match[] = {
	{
		.compatible = "samsung,exynos-usbdrd-phy",
		.data = &exynos_usbdrd_phy
	},
	{ },
};
MODULE_DEVICE_TABLE(of, exynos_usbdrd_phy_of_match);

static int exynos_usbdrd_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct exynos_usbdrd_phy *phy_drd;
	struct phy_provider *phy_provider;
	struct resource *res;
	const struct of_device_id *match;
	const struct exynos_usbdrd_phy_drvdata *drv_data;
	struct regmap *reg_pmu;
	u32 pmu_offset, pmu_offset_dp, pmu_mask;
	int i, ret;

	pr_info("%s: +++ %s %s\n", __func__, dev->init_name, pdev->name);
	phy_drd = devm_kzalloc(dev, sizeof(*phy_drd), GFP_KERNEL);
	if (!phy_drd)
		return -ENOMEM;

	dev_info(dev, "Get USB LDO!\n");
	phy_drd->vdd075_usb = regulator_get(dev, "vdd075_usb");
	if (IS_ERR(phy_drd->vdd075_usb) || phy_drd->vdd075_usb == NULL) {
		dev_err(dev, "%s - vdd075_usb regulator_get fail %p %d\n",
			__func__, phy_drd->vdd075_usb, IS_ERR(phy_drd->vdd075_usb));
	}

	phy_drd->vdd18_usb = regulator_get(dev, "vdd18_usb");
	if (IS_ERR(phy_drd->vdd18_usb) || phy_drd->vdd18_usb == NULL) {
		dev_err(dev, "%s - vdd18_usb regulator_get fail %p %d\n",
			__func__, phy_drd->vdd18_usb, IS_ERR(phy_drd->vdd18_usb));
	}

	phy_drd->vdd33_usb = regulator_get(dev, "vdd33_usb");
	if (IS_ERR(phy_drd->vdd33_usb) || phy_drd->vdd33_usb == NULL) {
		dev_err(dev, "%s - vdd33_usb regulator_get fail %p %d\n",
				__func__, phy_drd->vdd33_usb, IS_ERR(phy_drd->vdd33_usb));
		return -EPROBE_DEFER;
	}

	dev_set_drvdata(dev, phy_drd);
	phy_drd->dev = dev;

	match = of_match_node(exynos_usbdrd_phy_of_match, pdev->dev.of_node);

	drv_data = match->data;
	phy_drd->drv_data = drv_data;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	phy_drd->reg_phy = devm_ioremap_resource(dev, res);
	if (IS_ERR(phy_drd->reg_phy))
		return PTR_ERR(phy_drd->reg_phy);

	ret = exynos_usbdrd_clk_get(phy_drd);
	if (ret) {
		dev_err(dev, "%s: Failed to get clocks\n", __func__);
		goto skip_clock;
	}

	ret = exynos_usbdrd_clk_prepare(phy_drd);
	if (ret) {
		dev_err(dev, "%s: Failed to prepare clocks\n", __func__);
		return ret;
	}

	ret = exynos_rate_to_clk(phy_drd);
	if (ret) {
		dev_err(phy_drd->dev, "%s: Not supported ref clock\n",
				__func__);
		goto err1;
	}
skip_clock:

	reg_pmu = syscon_regmap_lookup_by_phandle(dev->of_node,
						   "samsung,pmu-syscon");
	if (IS_ERR(reg_pmu)) {
		dev_err(dev, "Failed to lookup PMU regmap\n");
		goto err1;
	}

	ret = of_property_read_u32(dev->of_node, "pmu_offset", &pmu_offset);
	if (ret < 0) {
		dev_err(dev, "couldn't read pmu_offset on %s node, error = %d\n",
						dev->of_node->name, ret);
		goto err1;
	}
	ret = of_property_read_u32(dev->of_node, "pmu_offset_dp", &pmu_offset_dp);
	if (ret < 0) {
		dev_err(dev, "couldn't read pmu_offset_dp on %s node, error = %d\n",
						dev->of_node->name, ret);
		goto err1;
	}
	ret = of_property_read_u32(dev->of_node, "pmu_mask", &pmu_mask);
	if (ret < 0) {
		dev_err(dev, "couldn't read pmu_mask on %s node, error = %d\n",
						dev->of_node->name, ret);
		goto err1;
	}

	pmu_mask = (u32)BIT(pmu_mask);

	ret = exynos_usbdrd_get_phyinfo(phy_drd);
	if (ret)
		goto err1;

	for (i = 0; i < EXYNOS_DRDPHYS_NUM; i++) {
		struct phy *phy = devm_phy_create(dev, NULL,
						  &exynos_usbdrd_phy_ops);
		if (IS_ERR(phy)) {
			dev_err(dev, "Failed to create usbdrd_phy phy\n");
			goto err1;
		}

		phy_drd->phys[i].phy = phy;
		phy_drd->phys[i].index = i;
		phy_drd->phys[i].reg_pmu = reg_pmu;
		phy_drd->phys[i].pmu_offset = pmu_offset;
		phy_drd->phys[i].pmu_offset_dp = pmu_offset_dp;
		phy_drd->phys[i].pmu_mask = pmu_mask;
		phy_drd->phys[i].phy_cfg = &drv_data->phy_cfg[i];
		phy_set_drvdata(phy, &phy_drd->phys[i]);
	}

	phy_provider = devm_of_phy_provider_register(dev,
						     exynos_usbdrd_phy_xlate);
	if (IS_ERR(phy_provider)) {
		goto err1;
	}

	spin_lock_init(&phy_drd->lock);

	pr_info("%s: ---\n", __func__);
	return 0;
err1:
	exynos_usbdrd_clk_unprepare(phy_drd);

	return ret;
}

static struct platform_driver phy_exynos_usbdrd = {
	.probe	= exynos_usbdrd_phy_probe,
	.driver = {
		.of_match_table	= exynos_usbdrd_phy_of_match,
		.name		= "phy_exynos_usbdrd",
	}
};

module_platform_driver(phy_exynos_usbdrd);

MODULE_SOFTDEP("post:dwc3-exynos-usb");
MODULE_DESCRIPTION("Samsung EXYNOS SoCs USB DRD controller PHY driver");
MODULE_AUTHOR("Vivek Gautam <gautam.vivek@samsung.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:phy_exynos_usbdrd");
