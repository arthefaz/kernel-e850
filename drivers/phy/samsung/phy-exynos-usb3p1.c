// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com
 *
 * Author: Sung-Hyun Na <sunghyun.na@samsung.com>
 *
 * Chip Abstraction Layer for USB PHY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include "phy-samsung-usb-cal.h"
#include "phy-exynos-usb3p1.h"
#include "phy-exynos-usb3p1-reg.h"

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
	int main_version;
	u32 clkrst;

	main_version = info->version & EXYNOS_USBCON_VER_MAJOR_VER_MASK;
	if ((main_version == EXYNOS_USBCON_VER_05_0_0) &&
			(info->used_phy_port == 1)) {
		regs_base = info->regs_base_2nd;
	}

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
	int main_version;
	u32 clkrst;

	main_version = info->version & EXYNOS_USBCON_VER_MAJOR_VER_MASK;
	if ((main_version == EXYNOS_USBCON_VER_05_0_0) &&
			(info->used_phy_port == 1))
		regs_base = info->regs_base_2nd;

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

void phy_exynos_usb_v3p1_pma_ready(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;

	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg &= ~PMA_LOW_PWR;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);

	udelay(1);

	reg |= PMA_APB_SW_RST;
	reg |= PMA_INIT_SW_RST;
	reg |= PMA_CMN_SW_RST;
	reg |= PMA_TRSV_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);

	udelay(1);

	reg &= ~PMA_APB_SW_RST;
	reg &= ~PMA_INIT_SW_RST;
	reg &= ~PMA_PLL_REF_REQ_MASK;
	reg &= ~PMA_REF_FREQ_SEL_MASK;
	reg |= PMA_REF_FREQ_SEL_SET(0x1);
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);

	reg = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
	reg &= ~LINKCTRL_PIPE3_FORCE_EN;
	writel(reg, regs_base + EXYNOS_USBCON_LINK_CTRL);
}

void phy_exynos_usb_v3p1_g2_pma_ready(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;

	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg &= ~PMA_ROPLL_REF_CLK_SEL_MASK;
	reg &= ~PMA_LCPLL_REF_CLK_SEL_MASK;
	reg &= ~PMA_PLL_REF_REQ_MASK;
	reg |= PMA_REF_FREQ_SEL_SET(1);
	reg |= PMA_LOW_PWR;
	reg |= PMA_TRSV_SW_RST;
	reg |= PMA_CMN_SW_RST;
	reg |= PMA_INIT_SW_RST;
	reg |= PMA_APB_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);

	udelay(1);
	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg &= ~PMA_LOW_PWR;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	udelay(1);

	// release overide
	reg = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
	reg &= ~LINKCTRL_PIPE3_FORCE_EN;
	writel(reg, regs_base + EXYNOS_USBCON_LINK_CTRL);

	udelay(1);

	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg &= ~PMA_APB_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
}

void phy_exynos_usb_v3p1_g2_disable(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;

	/* Change pipe pclk to pipe3 */
	reg = readl(regs_base + EXYNOS_USBCON_CLKRST);
	reg &= ~CLKRST_LINK_PCLK_SEL;
	writel(reg, regs_base + EXYNOS_USBCON_CLKRST);

	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg |= PMA_LOW_PWR;
	reg |= PMA_TRSV_SW_RST;
	reg |= PMA_CMN_SW_RST;
	reg |= PMA_INIT_SW_RST;
	reg |= PMA_APB_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);

	udelay(1);

	reg &= ~PMA_TRSV_SW_RST;
	reg &= ~PMA_CMN_SW_RST;
	reg &= ~PMA_INIT_SW_RST;
	reg &= ~PMA_APB_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
}

void phy_exynos_usb_v3p1_pma_sw_rst_release(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;

	/* Reset Release for USB/DP PHY */
	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg &= ~PMA_CMN_SW_RST;
	reg &= ~PMA_TRSV_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);

	udelay(1000);

	/* Change pipe pclk to pipe3 */
	reg = readl(regs_base + EXYNOS_USBCON_CLKRST);
	reg |= CLKRST_LINK_PCLK_SEL;
	writel(reg, regs_base + EXYNOS_USBCON_CLKRST);
}

void phy_exynos_usb_v3p1_g2_pma_sw_rst_release(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;

	/* Reset Release for USB/DP PHY */
	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg &= ~PMA_INIT_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);

	udelay(1); // Spec : wait for 200ns

	/* run pll */
	reg = readl(regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);
	reg &= ~PMA_TRSV_SW_RST;
	reg &= ~PMA_CMN_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_COMBO_PMA_CTRL);

	udelay(10);
}

void phy_exynos_usb_v3p1_g2_link_pclk_sel(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;

	/* Change pipe pclk to pipe3 */
	/* add by Makalu(evt1) case - 20180724 */
	reg = readl(regs_base + EXYNOS_USBCON_CLKRST);
	reg |= CLKRST_LINK_PCLK_SEL;
	writel(reg, regs_base + EXYNOS_USBCON_CLKRST);
	mdelay(3);
}

void phy_exynos_usb_v3p1_pipe_ready(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 reg;

	reg = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
	reg &= ~LINKCTRL_PIPE3_FORCE_EN;
	writel(reg, regs_base + EXYNOS_USBCON_LINK_CTRL);
}

void phy_exynos_usb_v3p1_pipe_ovrd(struct exynos_usbphy_info *info)
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

void phy_exynos_usb_v3p1_link_sw_reset(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	int main_version;
	u32 reg;

	main_version = info->version & EXYNOS_USBCON_VER_MAJOR_VER_MASK;

	/* use link_sw_rst because it has functioning as Hreset_n
	 * for asb host/device role change, originally not recommend link_sw_rst by Foundry T.
	 * so that some of global register has cleard - 2018.11.12
	 */
	//#ifndef __BOOT__
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

void phy_exynos_usb_v3p1_enable(struct exynos_usbphy_info *info)
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

	if (info->common_block_disable) {
		reg |= HSP_EN_UTMISUSPEND;
		reg |= HSP_COMMONONN;
	} else {
		reg &= ~HSP_COMMONONN;
	}
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

	/* Select PHY MUX */
	if (info->dual_phy) {
		u32 physel;

		physel = readl(regs_base + EXYNOS_USBCON_DUALPHYSEL);
		if (info->used_phy_port == 0) {
			physel &= ~DUALPHYSEL_PHYSEL_CTRL;
			physel &= ~DUALPHYSEL_PHYSEL_SSPHY;
			physel &= ~DUALPHYSEL_PHYSEL_PIPECLK;
			physel &= ~DUALPHYSEL_PHYSEL_PIPERST;
		} else {
			physel |= DUALPHYSEL_PHYSEL_CTRL;
			physel |= DUALPHYSEL_PHYSEL_SSPHY;
			physel |= DUALPHYSEL_PHYSEL_PIPECLK;
			physel |= DUALPHYSEL_PHYSEL_PIPERST;
		}
		writel(physel, regs_base + EXYNOS_USBCON_DUALPHYSEL);
	}

	/* 2. OVC io usage */
	reg = readl(regs_base + EXYNOS_USBCON_LINK_PORT);
	if (info->use_io_for_ovc) {
		reg &= ~LINKPORT_HUB_PORT_SEL_OCD_U3;
		reg &= ~LINKPORT_HUB_PORT_SEL_OCD_U2;
	} else {
		reg |= LINKPORT_HUB_PORT_SEL_OCD_U3;
		reg |= LINKPORT_HUB_PORT_SEL_OCD_U2;
	}
	writel(reg, regs_base + EXYNOS_USBCON_LINK_PORT);
}

enum exynos_usbcon_cr {
	USBCON_CR_ADDR = 0,
	USBCON_CR_DATA = 1,
	USBCON_CR_READ = 18,
	USBCON_CR_WRITE = 19,
};

//For KITT
enum exynos_usbcon_ssp_cr {
	USBCON_CR_SSP_READ = 18,
	USBCON_CR_SSP_WRITE = 19,
};

enum exynos_usbcon_ssp_cr_clk {
	CR_CLK_HIGH = 0x1,
	CR_CLK_LOW = 0x0,
};

static u16 phy_exynos_usb_v3p1_cr_access(struct exynos_usbphy_info *info,
	enum exynos_usbcon_cr cr_bit, u16 data)
{
	void __iomem *base;

	u32 ssp_crctl0 = 0;
	u32 ssp_crctl1 = 0;
	u32 loop;
	u32 loop_cnt;

	if (info->used_phy_port != -1) {
		if (info->used_phy_port == 0)
			base = info->regs_base;
		else
			base = info->regs_base_2nd;
	} else
		base = info->regs_base;

	/*Clear CR port register*/
	ssp_crctl0 = readl(base + EXYNOS_USBCON_SSP_CRCTL0);
	ssp_crctl0 &= ~(0xf);
	ssp_crctl0 &= ~(0xffffU << 16);
	writel(ssp_crctl0, base + EXYNOS_USBCON_SSP_CRCTL0);

	/*Set Data for cr port*/
	ssp_crctl0 &= ~SSP_CCTRL0_CR_DATA_IN_MASK;
	ssp_crctl0 |= SSP_CCTRL0_CR_DATA_IN(data);
	writel(ssp_crctl0, base + EXYNOS_USBCON_SSP_CRCTL0);

	if (cr_bit == USBCON_CR_ADDR)
		loop = 1;
	else
		loop = 2;

	for (loop_cnt = 0; loop_cnt < loop; loop_cnt++) {
		u32 trigger_bit = 0;
		u32 handshake_cnt = 2;
		/* Trigger cr port */
		if (cr_bit == USBCON_CR_ADDR)
			trigger_bit = SSP_CRCTRL0_CR_CAP_ADDR;
		else {
			if (loop_cnt == 0)
				trigger_bit = SSP_CRCTRL0_CR_CAP_DATA;
			else {
				if (cr_bit == USBCON_CR_READ)
					trigger_bit = SSP_CRCTRL0_CR_READ;
				else
					trigger_bit = SSP_CRCTRL0_CR_WRITE;
			}
		}
		/* Handshake Procedure */
		do {
			u32 usec = 100;
			if (handshake_cnt == 2)
				ssp_crctl0 |= trigger_bit;
			else
				ssp_crctl0 &= ~trigger_bit;
			writel(ssp_crctl0, base + EXYNOS_USBCON_SSP_CRCTL0);

			/* Handshake */
			do {
				ssp_crctl1 = readl(base + EXYNOS_USBCON_SSP_CRCTL1);
				if ((handshake_cnt == 2) && (ssp_crctl1 & SSP_CRCTL1_CR_ACK))
					break;
				else if ((handshake_cnt == 1) && !(ssp_crctl1 & SSP_CRCTL1_CR_ACK))
					break;
				else
					udelay(1);
			} while (usec-- > 0);

			if (!usec)
				pr_err("CRPORT handshake timeout1 (0x%08x)\n", ssp_crctl0);

			udelay(5);
			handshake_cnt--;
		} while (handshake_cnt != 0);
		udelay(50);

	}
	return (u16) ((ssp_crctl1 & SSP_CRCTL1_CR_DATA_OUT_MASK) >> 16);
}

//For KITT
static u32 phy_exynos_usb_v3p1_ssp_cr_clk_up(struct exynos_usbphy_info *info)
{
	void __iomem *base = info->regs_base;
	u32 g2phy_crparcon0 = 0;

	udelay(10);
	g2phy_crparcon0 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON0);
	g2phy_crparcon0 |= G2PHY_CRPARCON0_CR_PARA_CLK;
	writel(g2phy_crparcon0, base + EXYNOS_USBCON_G2PHY_CRPARCON0);

	return CR_CLK_HIGH;
}

//For KITT
static u32 phy_exynos_usb_v3p1_ssp_cr_clk_down(struct exynos_usbphy_info *info)
{
	void __iomem *base = info->regs_base;
	u32 g2phy_crparcon0 = 0;

	udelay(10);
	g2phy_crparcon0 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON0);
	g2phy_crparcon0 &= ~(G2PHY_CRPARCON0_CR_PARA_CLK);
	writel(g2phy_crparcon0, base + EXYNOS_USBCON_G2PHY_CRPARCON0);

	return CR_CLK_LOW;
}

//For KITT
static void phy_exynos_usb_v3p1_ssp_cr_clk(struct exynos_usbphy_info *info)
{
	phy_exynos_usb_v3p1_ssp_cr_clk_up(info);
	phy_exynos_usb_v3p1_ssp_cr_clk_down(info);
}

//For KITT------------------------------------------------------------------------
static u16 phy_exynos_usb_v3p1_ssp_cr_access(struct exynos_usbphy_info *info,
	enum exynos_usbcon_cr cr_op, u16 addr, u16 data)
{

	// DesignWare Cores USB 3.1 SS+ PHY for SS 10 LPP Databook
	// 5.12 Control Register Interface

	void __iomem *base;
	u32 PreClkCnt = 0x1;

	u32 g2phy_crparcon0 = 0;
	u32 g2phy_crparcon1 = 0;
	u32 g2phy_crparcon2 = 0;
	u32 loop_cnt = 0;

	base = info->regs_base;

	/*Clear CR port register*/
	/*Clear G2PHY_CRPARCON0*/
	g2phy_crparcon0 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON0);
	g2phy_crparcon0 &= ~(G2PHY_CRPARCON0_CR_PARA_ACK|G2PHY_CRPARCON0_CR_PARA_SEL
			|G2PHY_CRPARCON0_CR_PARA_CLK);
	g2phy_crparcon0 &= ~(0xffffU << 16);
	writel(g2phy_crparcon0, base + EXYNOS_USBCON_G2PHY_CRPARCON0);

	/*Clear G2PHY_CRPARCON1*/
	g2phy_crparcon1 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON1);
	g2phy_crparcon1 &= ~(G2PHY_CRPARCON1_CR_PARA_RD_EN);
	g2phy_crparcon1 &= ~(0xffffU << 16);
	writel(g2phy_crparcon1, base + EXYNOS_USBCON_G2PHY_CRPARCON1);

	/*Clear G2PHY_CRPARCON2*/
	g2phy_crparcon2 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON2);
	g2phy_crparcon2 &= ~(G2PHY_CRPARCON2_CR_PARA_WR_EN);
	g2phy_crparcon2 &= ~(0xffffU << 16);
	writel(g2phy_crparcon2, base + EXYNOS_USBCON_G2PHY_CRPARCON2);

	/*Set cr_para_sel to 0x1*/
	g2phy_crparcon0 |= G2PHY_CRPARCON0_CR_PARA_SEL;
	writel(g2phy_crparcon0, base + EXYNOS_USBCON_G2PHY_CRPARCON0);

	/*Pre clocking to PHY*/
	if (PreClkCnt != 0x0) {
		do {
			phy_exynos_usb_v3p1_ssp_cr_clk(info);
			PreClkCnt--;
			if (PreClkCnt == 0)
				break;
		} while (1);
	}

	do {
		if (loop_cnt == 0)
			phy_exynos_usb_v3p1_ssp_cr_clk(info);
		else if (loop_cnt == 1) {
			g2phy_crparcon0 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON0);
			g2phy_crparcon0 |= (G2PHY_CRPARCON0_CR_PARA_ADDR(addr)|(G2PHY_CRPARCON0_CR_PARA_CLK));

			if (cr_op == USBCON_CR_WRITE) {
				g2phy_crparcon2 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON2);
				g2phy_crparcon2 |= (G2PHY_CRPARCON2_CR_PARA_WR_DATA(data)|G2PHY_CRPARCON2_CR_PARA_WR_EN);

				phy_exynos_usb_v3p1_ssp_cr_clk_up(info);	// Toggle High
				writel(g2phy_crparcon0, base + EXYNOS_USBCON_G2PHY_CRPARCON0);	// write addr
				writel(g2phy_crparcon2, base + EXYNOS_USBCON_G2PHY_CRPARCON2);  // write Data + EN

			} else {
				// cr_op==USBCON_CR_READ
				g2phy_crparcon1 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON1);
				g2phy_crparcon1 |= G2PHY_CRPARCON1_CR_PARA_RD_EN;

				g2phy_crparcon0 |= phy_exynos_usb_v3p1_ssp_cr_clk_up(info);	// Toggle High
				writel(g2phy_crparcon0, base + EXYNOS_USBCON_G2PHY_CRPARCON0);	// read Addr
				writel(g2phy_crparcon1, base + EXYNOS_USBCON_G2PHY_CRPARCON1);	// read EN: High
			}

			g2phy_crparcon0 &= ~(0x1 << phy_exynos_usb_v3p1_ssp_cr_clk_down(info));	// Toggle Low

		} else if (loop_cnt == 2) {
			if (cr_op == USBCON_CR_WRITE) {
				//g2phy_crparcon2 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON2);
				//g2phy_crparcon2 &= ~(G2PHY_CRPARCON2_CR_PARA_WR_EN);

				//phy_exynos_usb_v3p1_ssp_cr_clk_up(info);	// Toggle High
				//writel(g2phy_crparcon2, base + EXYNOS_USBCON_G2PHY_CRPARCON2);
			} else {
				// cr_op==USBCON_CR_READ
				g2phy_crparcon1 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON1);
				g2phy_crparcon1 &= ~(G2PHY_CRPARCON1_CR_PARA_RD_EN);

				g2phy_crparcon0 |= phy_exynos_usb_v3p1_ssp_cr_clk_up(info);	// Toggle High
				writel(g2phy_crparcon1, base + EXYNOS_USBCON_G2PHY_CRPARCON1); // read EN: Low
			}

			g2phy_crparcon0 &= ~(0x1 << phy_exynos_usb_v3p1_ssp_cr_clk_down(info));	// Toggle Low

		} else {
			// loop_cnt > 2
			g2phy_crparcon0 |= phy_exynos_usb_v3p1_ssp_cr_clk_up(info);	// Toggle High and update
			g2phy_crparcon0 = G2PHY_CRPARCON0_CR_PARA_ACK_GET(readl(base + EXYNOS_USBCON_G2PHY_CRPARCON0));
			if (g2phy_crparcon0) {
				g2phy_crparcon1 = readl(base + EXYNOS_USBCON_G2PHY_CRPARCON1);
				break;
			}
			g2phy_crparcon0 &= ~(0x1 << phy_exynos_usb_v3p1_ssp_cr_clk_down(info));	// Toggle Low and update
		}

		loop_cnt++;

		if (loop_cnt == 0x1000)
			break;
	} while (1);

	return (u16) ((g2phy_crparcon1 & (0xffffU << 16)) >> 16);
}

void phy_exynos_usb_v3p1_cal_cr_write(struct exynos_usbphy_info *info, u16 addr, u16 data)
{
	phy_exynos_usb_v3p1_cr_access(info, USBCON_CR_ADDR, addr);
	phy_exynos_usb_v3p1_cr_access(info, USBCON_CR_WRITE, data);
}

u16 phy_exynos_usb_v3p1_cal_cr_read(struct exynos_usbphy_info *info, u16 addr)
{
	phy_exynos_usb_v3p1_cr_access(info, USBCON_CR_ADDR, addr);
	return phy_exynos_usb_v3p1_cr_access(info, USBCON_CR_READ, 0);
}

//For KITT ---------------------------------------------------------------------------------------
void phy_exynos_usb_v3p1_ssp_cal_cr_write(struct exynos_usbphy_info *info, u16 addr, u16 data)
{
	phy_exynos_usb_v3p1_ssp_cr_access(info, USBCON_CR_WRITE, addr, data);
}
//--------------------------------------------------------------------------------------------

//For KITT ---------------------------------------------------------------------------------------
u16 phy_exynos_usb_v3p1_ssp_cal_cr_read(struct exynos_usbphy_info *info, u16 addr)
{
	return phy_exynos_usb_v3p1_ssp_cr_access(info, USBCON_CR_READ, addr, 0);

}
//--------------------------------------------------------------------------------------------

enum exynos_usbphy_tif {
	USBCON_TIF_RD_STS,
	USBCON_TIF_RD_OVRD,
	USBCON_TIF_WR_OVRD,
};

static u8 phy_exynos_usb_v3p1_tif_access(struct exynos_usbphy_info *info,
	enum exynos_usbphy_tif access_type, u8 addr, u8 data)
{
	void __iomem *base;
	u32 hsp_test;

	base = info->regs_base;
	hsp_test = readl(base + EXYNOS_USBCON_HSP_TEST);
	/* Set TEST DATA OUT SEL */
	if (access_type == USBCON_TIF_RD_STS)
		hsp_test &= ~HSP_TEST_DATA_OUT_SEL;
	else
		hsp_test |= HSP_TEST_DATA_OUT_SEL;
	hsp_test &= ~HSP_TEST_DATA_IN_MASK;
	hsp_test &= ~HSP_TEST_DATA_ADDR_MASK;
	hsp_test |= HSP_TEST_DATA_ADDR_SET(addr);
	writel(hsp_test, base + EXYNOS_USBCON_HSP_TEST);

	udelay(10);

	hsp_test = readl(base + EXYNOS_USBCON_HSP_TEST);
	if (access_type != USBCON_TIF_WR_OVRD)
		return HSP_TEST_DATA_OUT_GET(hsp_test);

	hsp_test |= HSP_TEST_DATA_IN_SET((data | 0xf0));
	hsp_test |= HSP_TEST_CLK;
	writel(hsp_test, base + EXYNOS_USBCON_HSP_TEST);

	udelay(10);

	hsp_test = readl(base + EXYNOS_USBCON_HSP_TEST);
	hsp_test &= ~HSP_TEST_CLK;
	writel(hsp_test, base + EXYNOS_USBCON_HSP_TEST);

	hsp_test = readl(base + EXYNOS_USBCON_HSP_TEST);
	return HSP_TEST_DATA_OUT_GET(hsp_test);
}

u8 phy_exynos_usb_v3p1_tif_ov_rd(struct exynos_usbphy_info *info, u8 addr)
{
	return phy_exynos_usb_v3p1_tif_access(info, USBCON_TIF_RD_OVRD, addr, 0x0);
}

u8 phy_exynos_usb_v3p1_tif_ov_wr(struct exynos_usbphy_info *info, u8 addr, u8 data)
{
	return phy_exynos_usb_v3p1_tif_access(info, USBCON_TIF_WR_OVRD, addr, data);
}

u8 phy_exynos_usb_v3p1_tif_sts_rd(struct exynos_usbphy_info *info, u8 addr)
{
	return phy_exynos_usb_v3p1_tif_access(info, USBCON_TIF_RD_STS, addr, 0x0);
}

void phy_exynos_usb_v3p1_disable(struct exynos_usbphy_info *info)
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

	/* link sw reset is need for USB_DP/DM high-z in host mode: 2019.04.10 by daeman.ko*/
	phy_exynos_usb_v3p1_link_sw_reset(info);
}

u64 phy_exynos_usb3p1_get_logic_trace(struct exynos_usbphy_info *info)
{
	u64 ret;
	void __iomem *regs_base = info->regs_base;

	ret = readl(regs_base + EXYNOS_USBCON_LINK_DEBUG_L);
	ret |= ((u64) readl(regs_base + EXYNOS_USBCON_LINK_DEBUG_H)) << 32;

	return ret;
}

u8 phy_exynos_usb3p1_get_phyclkcnt(struct exynos_usbphy_info *info)
{
	u32 hsp_test;
	u8 phyclkcnt;
	void __iomem *regs_base = info->regs_base;

	/* phyclkcnt
	 * HSP_TEST.[31:26] phyclkcnt is top 6bit of [29:0] phyclkcnt.
	 * 1 count is about 280ms.
	 */
	hsp_test = readl(regs_base + EXYNOS_USBCON_HSP_TEST);
	phyclkcnt = HSP_TEST_PHYCLKCNT_GET(hsp_test);

	return phyclkcnt;
}

u8 phy_exynos_usb3p1_get_linestate(struct exynos_usbphy_info *info)
{
	u32 hsp_test;
	u8 linestate;
	void __iomem *regs_base = info->regs_base;

	hsp_test = readl(regs_base + EXYNOS_USBCON_HSP_TEST);
	linestate = HSP_TEST_LINESTATE_GET(hsp_test);

	return linestate;
}

void phy_exynos_usb3p1_pcs_reset(struct exynos_usbphy_info *info)
{
	u32 clkrst;
	void __iomem *regs_base = info->regs_base;

	clkrst = readl(regs_base + EXYNOS_USBCON_CLKRST);
	if (EXYNOS_USBCON_VER_MINOR(info->version) >= 0x1) {
		/* TODO : How to pcs reset
		 * clkrst |= CLKRST_PHY30_RST_SEL;
		 * clkrst |= CLKRST_PHY30_SW_RST;
		 */
	} else {
		clkrst |= CLKRST_PHY_SW_RST;
		clkrst |= CLKRST_PHY_RST_SEL;
	}
	writel(clkrst, regs_base + EXYNOS_USBCON_CLKRST);

	udelay(1);

	clkrst = readl(regs_base + EXYNOS_USBCON_CLKRST);
	if (EXYNOS_USBCON_VER_MINOR(info->version) >= 0x1) {
		clkrst |= CLKRST_PHY30_RST_SEL;
		clkrst &= ~CLKRST_PHY30_SW_RST;
	} else {
		clkrst |= CLKRST_PHY_RST_SEL;
		clkrst &= ~CLKRST_PHY_SW_RST;
	}
	writel(clkrst, regs_base + EXYNOS_USBCON_CLKRST);
}

void phy_exynos_usb_v3p1_enable_dp_pullup(
		struct exynos_usbphy_info *usbphy_info)
{
	void __iomem *regs_base = usbphy_info->regs_base;
	u32 phyutmi;

	phyutmi = readl(regs_base + EXYNOS_USBCON_HSP);
	phyutmi |= HSP_VBUSVLDEXT;
	writel(phyutmi, regs_base + EXYNOS_USBCON_HSP);
}

void phy_exynos_usb_v3p1_disable_dp_pullup(
		struct exynos_usbphy_info *usbphy_info)
{
	void __iomem *regs_base = usbphy_info->regs_base;
	u32 phyutmi;

	phyutmi = readl(regs_base + EXYNOS_USBCON_HSP);
	phyutmi &= ~HSP_VBUSVLDEXT;
	writel(phyutmi, regs_base + EXYNOS_USBCON_HSP);
}

void phy_exynos_usb_v3p1_config_host_mode(struct exynos_usbphy_info *info)
{
	u32 reg;
	void __iomem *regs_base = info->regs_base;

	/* DP/DM Pull Down Control */
	reg = readl(regs_base + EXYNOS_USBCON_UTMI);
	reg |= UTMI_DP_PULLDOWN;
	reg |= UTMI_DM_PULLDOWN;
	reg &= ~UTMI_FORCE_BVALID;
	reg &= ~UTMI_FORCE_VBUSVALID;
	writel(reg, regs_base + EXYNOS_USBCON_UTMI);

	/* Disable Pull-up Register */
	reg = readl(regs_base + EXYNOS_USBCON_HSP);
	reg &= ~HSP_VBUSVLDEXTSEL;
	reg &= ~HSP_VBUSVLDEXT;
	writel(reg, regs_base + EXYNOS_USBCON_HSP);
}

void phy_exynos_usb3p1_set_fs_vplus_vminus(
		struct exynos_usbphy_info *usbphy_info, u32 fsls_speed_sel, u32 fsv_out_en)
{
	void __iomem *regs_base = usbphy_info->regs_base;
	u32 hsp_ctrl;

	if (fsv_out_en) {
		hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
		if (fsls_speed_sel)
			hsp_ctrl |= HSP_FSLS_SPEED_SEL;
		else
			hsp_ctrl &= ~HSP_FSLS_SPEED_SEL;
		hsp_ctrl |= HSP_FSVP_OUT_EN;
		hsp_ctrl |= HSP_FSVM_OUT_EN;
		writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);
	} else {
		hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
		hsp_ctrl &= ~HSP_FSLS_SPEED_SEL;
		hsp_ctrl &= ~HSP_FSVP_OUT_EN;
		hsp_ctrl &= ~HSP_FSVM_OUT_EN;
		writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);
	}

}

u8 phy_exynos_usb3p1_get_fs_vplus_vminus(struct exynos_usbphy_info *info)
{
	void __iomem *regs_base = info->regs_base;
	u32 hsp_ctrl;
	u32 fsvplus, fsvminus;

	hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
	fsvplus = HSP_FSVPLUS_GET(hsp_ctrl);
	fsvminus = HSP_FSVMINUS_GET(hsp_ctrl);

	return ((fsvminus & 0x1) << 1) | (fsvplus & 0x1);
}

void phy_exynos_usb3p1_set_fsv_out_en(
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

u8 phy_exynos_usb3p1_bc_data_contact_detect(struct exynos_usbphy_info *usbphy_info)
{
	bool ret = false;
	u32 utmi_ctrl, hsp_ctrl;
	u32 cnt;
	u32 fsvplus;
	void __iomem *regs_base = usbphy_info->regs_base;

	// set UTMI_CTRL
	utmi_ctrl = readl(regs_base + EXYNOS_USBCON_UTMI);
	utmi_ctrl |= UTMI_OPMODE_CTRL_EN;
	utmi_ctrl &= ~UTMI_FORCE_OPMODE_MASK;
	utmi_ctrl |= UTMI_FORCE_OPMODE_SET(1);
	utmi_ctrl &= ~UTMI_DP_PULLDOWN;
	utmi_ctrl |= UTMI_DM_PULLDOWN;
	utmi_ctrl |= UTMI_FORCE_SUSPEND;
	writel(utmi_ctrl, regs_base + EXYNOS_USBCON_UTMI);

	// Data contact Detection Enable
	hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
	hsp_ctrl &= ~HSP_VDATSRCENB;
	hsp_ctrl &= ~HSP_VDATDETENB;
	hsp_ctrl |= HSP_DCDENB;
	writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);

	for (cnt = 8; cnt != 0; cnt--) {
		// TDCD_TIMEOUT, 300ms~900ms
		mdelay(40);

		hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
		fsvplus = HSP_FSVPLUS_GET(hsp_ctrl);

		if (!fsvplus)
			break;
	}

	if (fsvplus == 1 && cnt == 0)
		ret = false;
	else {
		mdelay(10);	// TDCD_DBNC, 10ms

		hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
		fsvplus = HSP_FSVPLUS_GET(hsp_ctrl);

		if (!fsvplus)
			ret = true;
		else
			ret = false;
	}

	hsp_ctrl &= ~HSP_DCDENB;
	writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);

	// restore UTMI_CTRL
	utmi_ctrl = readl(regs_base + EXYNOS_USBCON_UTMI);
	utmi_ctrl &= ~UTMI_OPMODE_CTRL_EN;
	utmi_ctrl &= ~UTMI_FORCE_OPMODE_MASK;
	utmi_ctrl &= ~UTMI_DM_PULLDOWN;
	utmi_ctrl &= ~UTMI_FORCE_SUSPEND;
	writel(utmi_ctrl, regs_base + EXYNOS_USBCON_UTMI);

	return ret;
}

enum exynos_usb_bc phy_exynos_usb3p1_bc_battery_charger_detection(struct exynos_usbphy_info *usbphy_info)
{
	u32 utmi_ctrl, hsp_ctrl;
	u32 chgdet;
	enum exynos_usb_bc chg_port = BC_SDP;
	void __iomem *regs_base = usbphy_info->regs_base;

	/** Step 1. Primary Detection :: SDP / DCP or CDP
	 * voltage sourcing on the D+ line and sensing on the D- line
	 **/
	// set UTMI_CTRL
	utmi_ctrl = readl(regs_base + EXYNOS_USBCON_UTMI);
	utmi_ctrl |= UTMI_OPMODE_CTRL_EN;
	utmi_ctrl &= ~UTMI_FORCE_OPMODE_MASK;
	utmi_ctrl |= UTMI_FORCE_OPMODE_SET(1);
	utmi_ctrl &= ~UTMI_DP_PULLDOWN;
	utmi_ctrl &= ~UTMI_DM_PULLDOWN;
	utmi_ctrl |= UTMI_FORCE_SUSPEND;
	writel(utmi_ctrl, regs_base + EXYNOS_USBCON_UTMI);

	hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
	hsp_ctrl &= ~HSP_CHRGSEL;
	hsp_ctrl |= HSP_VDATSRCENB;
	hsp_ctrl |= HSP_VDATDETENB;
	writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);

	mdelay(40);	// TVDMSRC_ON, 40ms

	hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
	chgdet = HSP_CHGDET_GET(hsp_ctrl);
	if (!chgdet) {
		/** IF CHGDET pin is not set,
		 * Standard Downstream Port
		 */
		chg_port = BC_SDP;
	} else {
		hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
		hsp_ctrl &= ~HSP_VDATSRCENB;
		hsp_ctrl &= ~HSP_VDATDETENB;
		writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);

		mdelay(20);

		/** ELSE Maybe DCP or CDP but DCP is primary charger */

		/** Step 2.1 Secondary Detection :: DCP or CDP
		 * voltage sourcing on the D- line and sensing on the D+ line
		 */
		hsp_ctrl |= HSP_CHRGSEL;
		hsp_ctrl |= HSP_VDATSRCENB;
		hsp_ctrl |= HSP_VDATDETENB;
		writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);

		mdelay(40);	// TVDMSRC_ON, 40ms

		hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
		chgdet = HSP_CHGDET_GET(hsp_ctrl);

		if (!chgdet)
			chg_port = BC_CDP;
		else
			chg_port = BC_DCP;
	}

	hsp_ctrl = readl(regs_base + EXYNOS_USBCON_HSP);
	hsp_ctrl &= ~HSP_VDATSRCENB;
	hsp_ctrl &= ~HSP_VDATDETENB;
	writel(hsp_ctrl, regs_base + EXYNOS_USBCON_HSP);

	// restore UTMI_CTRL
	utmi_ctrl = readl(regs_base + EXYNOS_USBCON_UTMI);
	utmi_ctrl &= ~UTMI_OPMODE_CTRL_EN;
	utmi_ctrl &= ~UTMI_FORCE_OPMODE_MASK;
	utmi_ctrl &= ~UTMI_FORCE_SUSPEND;
	writel(utmi_ctrl, regs_base + EXYNOS_USBCON_UTMI);

	return chg_port;
}
