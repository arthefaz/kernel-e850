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
#include <linux/of_device.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/soc/samsung/exynos-regs-pmu.h>

/* Exynos USB PHY registers */
#define EXYNOS_FSEL_9MHZ6		0x0
#define EXYNOS_FSEL_10MHZ		0x1
#define EXYNOS_FSEL_12MHZ		0x2
#define EXYNOS_FSEL_19MHZ2		0x1
#define EXYNOS_FSEL_20MHZ		0x4
#define EXYNOS_FSEL_24MHZ		0x5
#define EXYNOS_FSEL_26MHZ		0x82
#define EXYNOS_FSEL_50MHZ		0x7

#define EXYNOS_USBCON_LINK_CTRL			0x04
#define LINKCTRL_BUS_FILTER_BYPASS(_x)		((_x & 0xf) << 4)
#define LINKCTRL_FORCE_QACT			BIT(8)

#define EXYNOS_USBCON_CLKRST			0x20
#define CLKRST_LINK_SW_RST			BIT(0)
#define CLKRST_PORT_RST				BIT(1)
#define CLKRST_PHY30_SW_RST			BIT(3)

#define EXYNOS_USBCON_UTMI			0x50
#define UTMI_FORCE_SLEEP			BIT(0)
#define UTMI_FORCE_SUSPEND			BIT(1)
#define UTMI_DM_PULLDOWN			BIT(2)
#define UTMI_DP_PULLDOWN			BIT(3)
#define UTMI_FORCE_BVALID			BIT(4)
#define UTMI_FORCE_VBUSVALID			BIT(5)

#define EXYNOS_USBCON_HSP			0x54
#define HSP_COMMONONN				BIT(8)
#define HSP_EN_UTMISUSPEND			BIT(9)
#define HSP_VBUSVLDEXT				BIT(12)
#define HSP_VBUSVLDEXTSEL			BIT(13)
#define HSP_FSV_OUT_EN				BIT(24)

#define EXYNOS_USBCON_HSP_TEST			0x5c
#define HSP_TEST_SIDDQ				BIT(24)

#define KHZ	1000
#define MHZ	(KHZ * KHZ)

enum exynos_usbdrd_phy_id {
	EXYNOS_DRDPHY_UTMI,
	EXYNOS_DRDPHYS_NUM,
};

struct phy_usb_instance;
struct exynos_usbdrd_phy;

struct exynos_usbdrd_phy_config {
	u32 id;
	void (*phy_isol)(struct phy_usb_instance *inst, u32 on);
	void (*phy_init)(struct exynos_usbdrd_phy *phy_drd);
};

struct exynos_usbdrd_phy_drvdata {
	const struct exynos_usbdrd_phy_config *phy_cfg;
	u32 pmu_offset_usbdrd0_phy;
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
 */
struct exynos_usbdrd_phy {
	struct device *dev;
	void __iomem *reg_phy;
	struct clk *clk;
	const struct exynos_usbdrd_phy_drvdata *drv_data;
	struct phy_usb_instance {
		struct phy *phy;
		u32 index;
		struct regmap *reg_pmu;
		u32 pmu_offset;
		const struct exynos_usbdrd_phy_config *phy_cfg;
	} phys[EXYNOS_DRDPHYS_NUM];
	u32 extrefclk;
	struct clk *ref_clk;
	struct regulator *vbus;
};

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
static unsigned int exynos_rate_to_clk(unsigned long rate, u32 *reg)
{
	/* EXYNOS_FSEL_MASK */

	switch (rate) {
	case 9600 * KHZ:
		*reg = EXYNOS_FSEL_9MHZ6;
		break;
	case 10 * MHZ:
		*reg = EXYNOS_FSEL_10MHZ;
		break;
	case 12 * MHZ:
		*reg = EXYNOS_FSEL_12MHZ;
		break;
	case 19200 * KHZ:
		*reg = EXYNOS_FSEL_19MHZ2;
		break;
	case 20 * MHZ:
		*reg = EXYNOS_FSEL_20MHZ;
		break;
	case 24 * MHZ:
		*reg = EXYNOS_FSEL_24MHZ;
		break;
	case 26 * MHZ:
		*reg = EXYNOS_FSEL_26MHZ;
		break;
	case 50 * MHZ:
		*reg = EXYNOS_FSEL_50MHZ;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static void exynos_usbdrd_utmi_phy_isol(struct phy_usb_instance *inst,
					unsigned int on)
{
	unsigned int val;

	if (!inst->reg_pmu)
		return;

	val = on ? 0 : EXYNOS4_PHY_ENABLE;

	regmap_update_bits(inst->reg_pmu, inst->pmu_offset, EXYNOS4_PHY_ENABLE,
			   val);
}

static void exynos_usbdrd_utmi_init(struct exynos_usbdrd_phy *phy_drd)
{
	void __iomem *regs_base = phy_drd->reg_phy;
	u32 reg, reg_hsp;

	/* Disable HWACG */
	reg = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
	reg |= LINKCTRL_FORCE_QACT;
	writel(reg, regs_base + EXYNOS_USBCON_LINK_CTRL);

	/* Set PHY POR High */
	reg = readl(regs_base + EXYNOS_USBCON_CLKRST);
	reg |= CLKRST_PHY30_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_CLKRST);

	reg = readl(regs_base + EXYNOS_USBCON_UTMI);
	reg &= ~UTMI_FORCE_SUSPEND;
	reg &= ~UTMI_FORCE_SLEEP;
	reg &= ~UTMI_DP_PULLDOWN;
	reg &= ~UTMI_DM_PULLDOWN;
	writel(reg, regs_base + EXYNOS_USBCON_UTMI);

	/* Set phy clock & control HS phy */
	reg = readl(regs_base + EXYNOS_USBCON_HSP);
	reg |= HSP_EN_UTMISUSPEND;
	reg |= HSP_COMMONONN;
	writel(reg, regs_base + EXYNOS_USBCON_HSP);

	/*
	 * Follow setting sequence for USB Link
	 * 1. Set VBUS Valid and DP-Pull up control by VBUS pad usage
	 */
	reg = readl(regs_base + EXYNOS_USBCON_LINK_CTRL);
	reg |= LINKCTRL_BUS_FILTER_BYPASS(0xf);
	writel(reg, regs_base + EXYNOS_USBCON_LINK_CTRL);

	reg = readl(regs_base + EXYNOS_USBCON_UTMI);
	reg_hsp = readl(regs_base + EXYNOS_USBCON_HSP);
	reg |= UTMI_FORCE_BVALID;
	reg |= UTMI_FORCE_VBUSVALID;
	reg_hsp |= HSP_VBUSVLDEXTSEL;
	reg_hsp |= HSP_VBUSVLDEXT;
	writel(reg, regs_base + EXYNOS_USBCON_UTMI);
	writel(reg_hsp, regs_base + EXYNOS_USBCON_HSP);

	/* Enable PHY Power Mode */
	reg = readl(regs_base + EXYNOS_USBCON_HSP_TEST);
	reg &= ~HSP_TEST_SIDDQ;
	writel(reg, regs_base + EXYNOS_USBCON_HSP_TEST);

	/* Before POR low, 10us delay is needed. */
	udelay(10);

	/* Set PHY POR Low */
	reg = readl(regs_base + EXYNOS_USBCON_CLKRST);
	reg &= ~CLKRST_PHY30_SW_RST;
	reg &= ~CLKRST_PORT_RST;
	writel(reg, regs_base + EXYNOS_USBCON_CLKRST);

	/* After POR low and delay 75us, PHYCLOCK is guaranteed. */
	udelay(75);

	/* Disable UART/JTAG over USB */
	reg = readl(regs_base + EXYNOS_USBCON_HSP);
	reg &= ~HSP_FSV_OUT_EN;
	writel(reg, regs_base + EXYNOS_USBCON_HSP);
}

static int exynos_usbdrd_phy_init(struct phy *phy)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);
	int ret;

	ret = clk_prepare_enable(phy_drd->clk);
	if (ret)
		return ret;

	/* UTMI or PIPE3 specific init */
	inst->phy_cfg->phy_init(phy_drd);

	clk_disable_unprepare(phy_drd->clk);

	return 0;
}

static int exynos_usbdrd_phy_exit(struct phy *phy)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);
	void __iomem *regs_base = phy_drd->reg_phy;
	u32 reg;
	int ret;

	ret = clk_prepare_enable(phy_drd->clk);
	if (ret)
		return ret;

	/* Set phy clock & control HS phy */
	reg = readl(regs_base + EXYNOS_USBCON_UTMI);
	reg &= ~UTMI_DP_PULLDOWN;
	reg &= ~UTMI_DM_PULLDOWN;
	reg |= UTMI_FORCE_SUSPEND;
	reg |= UTMI_FORCE_SLEEP;
	writel(reg, regs_base + EXYNOS_USBCON_UTMI);

	/* Power down analog blocks */
	reg = readl(regs_base + EXYNOS_USBCON_HSP_TEST);
	reg |= HSP_TEST_SIDDQ;
	writel(reg, regs_base + EXYNOS_USBCON_HSP_TEST);

	/* Link reset */
	reg = readl(regs_base + EXYNOS_USBCON_CLKRST);
	reg |= CLKRST_LINK_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_CLKRST);
	udelay(10);
	reg &= ~CLKRST_LINK_SW_RST;
	writel(reg, regs_base + EXYNOS_USBCON_CLKRST);

	clk_disable_unprepare(phy_drd->clk);

	return 0;
}

static int exynos_usbdrd_phy_power_on(struct phy *phy)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	clk_prepare_enable(phy_drd->ref_clk);
	clk_prepare_enable(phy_drd->clk);

	inst->phy_cfg->phy_isol(inst, 0);

	return 0;
}

static int exynos_usbdrd_phy_power_off(struct phy *phy)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	inst->phy_cfg->phy_isol(inst, 1);

	clk_disable_unprepare(phy_drd->ref_clk);
	clk_disable_unprepare(phy_drd->clk);

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
	.owner		= THIS_MODULE,
};

static int exynos5_usbdrd_phy_clk_handle(struct exynos_usbdrd_phy *phy_drd)
{
	unsigned long ref_rate;
	int ret;

	phy_drd->clk = devm_clk_get(phy_drd->dev, "aclk");
	if (IS_ERR(phy_drd->clk)) {
		dev_err(phy_drd->dev, "couldn't get aclk clock\n");
		return PTR_ERR(phy_drd->clk);
	}

	phy_drd->ref_clk = devm_clk_get(phy_drd->dev, "ext_xtal");
	if (IS_ERR(phy_drd->ref_clk)) {
		dev_err(phy_drd->dev, "couldn't get ref clock\n");
		return PTR_ERR(phy_drd->ref_clk);
	}

	ref_rate = clk_get_rate(phy_drd->ref_clk);
	ret = exynos_rate_to_clk(ref_rate, &phy_drd->extrefclk);
	if (ret) {
		dev_err(phy_drd->dev, "Clock rate (%ld) not supported\n",
			ref_rate);
		return ret;
	}

	return 0;
}

static const struct exynos_usbdrd_phy_config phy_cfg_exynos850[] = {
	{
		.id		= EXYNOS_DRDPHY_UTMI,
		.phy_isol	= exynos_usbdrd_utmi_phy_isol,
		.phy_init	= exynos_usbdrd_utmi_init,
	},
};

static const struct exynos_usbdrd_phy_drvdata exynos850_usbdrd_phy = {
	.phy_cfg		= phy_cfg_exynos850,
	.pmu_offset_usbdrd0_phy	= EXYNOS5_USBDRD_PHY_CONTROL,
};

static const struct of_device_id exynos_usbdrd_phy_of_match[] = {
	{
		.compatible = "samsung,exynos850-usbdrd-phy",
		.data = &exynos850_usbdrd_phy
	},
	{ },
};
MODULE_DEVICE_TABLE(of, exynos_usbdrd_phy_of_match);

static int exynos_usbdrd_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct exynos_usbdrd_phy *phy_drd;
	struct phy_provider *phy_provider;
	const struct exynos_usbdrd_phy_drvdata *drv_data;
	struct regmap *reg_pmu;
	u32 pmu_offset;
	int i, ret;

	phy_drd = devm_kzalloc(dev, sizeof(*phy_drd), GFP_KERNEL);
	if (!phy_drd)
		return -ENOMEM;

	dev_set_drvdata(dev, phy_drd);
	phy_drd->dev = dev;

	phy_drd->reg_phy = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(phy_drd->reg_phy))
		return PTR_ERR(phy_drd->reg_phy);

	drv_data = of_device_get_match_data(dev);
	if (!drv_data)
		return -EINVAL;

	phy_drd->drv_data = drv_data;

	ret = exynos5_usbdrd_phy_clk_handle(phy_drd);
	if (ret) {
		dev_err(dev, "Failed to initialize clocks\n");
		return ret;
	}

	reg_pmu = syscon_regmap_lookup_by_phandle(dev->of_node,
						   "samsung,pmu-syscon");
	if (IS_ERR(reg_pmu)) {
		dev_err(dev, "Failed to lookup PMU regmap\n");
		return PTR_ERR(reg_pmu);
	}

	pmu_offset = phy_drd->drv_data->pmu_offset_usbdrd0_phy;

	phy_drd->vbus = devm_regulator_get(dev, "vbus");
	if (IS_ERR(phy_drd->vbus)) {
		ret = PTR_ERR(phy_drd->vbus);
		if (ret == -EPROBE_DEFER)
			return ret;

		dev_warn(dev, "Failed to get VBUS supply regulator\n");
		phy_drd->vbus = NULL;
	}

	for (i = 0; i < EXYNOS_DRDPHYS_NUM; i++) {
		struct phy *phy = devm_phy_create(dev, NULL,
						  &exynos_usbdrd_phy_ops);
		if (IS_ERR(phy)) {
			dev_err(dev, "Failed to create usbdrd_phy phy\n");
			return PTR_ERR(phy);
		}

		phy_drd->phys[i].phy = phy;
		phy_drd->phys[i].index = i;
		phy_drd->phys[i].reg_pmu = reg_pmu;
		phy_drd->phys[i].pmu_offset = pmu_offset;
		phy_drd->phys[i].phy_cfg = &drv_data->phy_cfg[i];
		phy_set_drvdata(phy, &phy_drd->phys[i]);
	}

	phy_provider = devm_of_phy_provider_register(dev,
						     exynos_usbdrd_phy_xlate);
	if (IS_ERR(phy_provider))
		return PTR_ERR(phy_provider);

	return 0;
}

static struct platform_driver phy_exynos_usbdrd = {
	.probe	= exynos_usbdrd_phy_probe,
	.driver = {
		.of_match_table	= exynos_usbdrd_phy_of_match,
		.name		= "phy_exynos_usbdrd",
		.suppress_bind_attrs = true,
	}
};

module_platform_driver(phy_exynos_usbdrd);
MODULE_DESCRIPTION("Samsung EXYNOS SoCs USB DRD controller PHY driver");
MODULE_AUTHOR("Vivek Gautam <gautam.vivek@samsung.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:phy_exynos_usbdrd");
