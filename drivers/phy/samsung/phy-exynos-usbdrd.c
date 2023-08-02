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
#include <linux/of_gpio.h>
#endif

#include "phy-exynos-usbdrd.h"

void __iomem *phycon_base_addr;
EXPORT_SYMBOL_GPL(phycon_base_addr);

struct usb_eom_result_s *eom_result;

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

static void exynos_usbdrd_pipe3_phy_isol(struct phy_usb_instance *inst,
					unsigned int on, unsigned int mask)
{
	unsigned int val;

	if (!inst->reg_pmu)
		return;

	val = on ? 0 : mask;

	regmap_update_bits(inst->reg_pmu, inst->pmu_offset_dp,
		mask, val);
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

	/* Control TCXO_BUF */
	if (inst->pmu_mask_tcxobuf != 0) {
		val = on ? 0 : inst->pmu_mask_tcxobuf;
		regmap_update_bits(inst->reg_pmu, inst->pmu_offset_tcxobuf,
			inst->pmu_mask_tcxobuf, val);
	}
}

/*
 * Sets the pipe3 phy's clk as EXTREFCLK (XXTI) which is internal clock
 * from clock core. Further sets multiplier values and spread spectrum
 * clock settings for SuperSpeed operations.
 */
static unsigned int
exynos_usbdrd_pipe3_set_refclk(struct phy_usb_instance *inst)
{
	static u32 reg;
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	/* PHYCLKRST setting isn't required in Combo PHY */
	if (phy_drd->usbphy_info.version >= EXYNOS_USBPHY_VER_02_0_0)
		return -EINVAL;

	/* restore any previous reference clock settings */
	reg = readl(phy_drd->reg_phy + EXYNOS_DRD_PHYCLKRST);

	/* Use EXTREFCLK as ref clock */
	reg &= ~PHYCLKRST_REFCLKSEL_MASK;
	reg |=	PHYCLKRST_REFCLKSEL_EXT_REFCLK;

	/* FSEL settings corresponding to reference clock */
	reg &= ~PHYCLKRST_FSEL_PIPE_MASK |
		PHYCLKRST_MPLL_MULTIPLIER_MASK |
		PHYCLKRST_SSC_REFCLKSEL_MASK;
	switch (phy_drd->extrefclk) {
	case EXYNOS_FSEL_50MHZ:
		reg |= (PHYCLKRST_MPLL_MULTIPLIER_50M_REF |
			PHYCLKRST_SSC_REFCLKSEL(0x00));
		break;
	case EXYNOS_FSEL_24MHZ:
		reg |= (PHYCLKRST_MPLL_MULTIPLIER_24MHZ_REF |
			PHYCLKRST_SSC_REFCLKSEL(0x88));
		break;
	case EXYNOS_FSEL_20MHZ:
		reg |= (PHYCLKRST_MPLL_MULTIPLIER_20MHZ_REF |
			PHYCLKRST_SSC_REFCLKSEL(0x00));
		break;
	case EXYNOS_FSEL_19MHZ2:
		reg |= (PHYCLKRST_MPLL_MULTIPLIER_19200KHZ_REF |
			PHYCLKRST_SSC_REFCLKSEL(0x88));
		break;
	default:
		dev_dbg(phy_drd->dev, "unsupported ref clk\n");
		break;
	}

	return reg;
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

	if (!of_property_read_u32(dev->of_node, "use_io_for_ovc", &value)) {
		phy_drd->usbphy_info.use_io_for_ovc = value ? true : false;
		} else {
		dev_err(dev, "can't get io_for_ovc\n");
		return -EINVAL;
		}

	if (!of_property_read_u32(dev->of_node, "common_block_disable", &value)) {
		phy_drd->usbphy_info.common_block_disable = value ? true : false;
	} else {
		dev_err(dev, "can't get common_block_disable\n");
		return -EINVAL;
	}

	phy_drd->usbphy_info.refclk = phy_drd->extrefclk;
	phy_drd->usbphy_info.regs_base = phy_drd->reg_phy;

	if (!of_property_read_u32(dev->of_node, "is_not_vbus_pad", &value)) {
		phy_drd->usbphy_info.not_used_vbus_pad = value ? true : false;
	} else {
		dev_err(dev, "can't get vbus_pad\n");
		return -EINVAL;
	}

	if (!of_property_read_u32(dev->of_node, "used_phy_port", &value)) {
		phy_drd->usbphy_info.used_phy_port = value ? true : false;
	} else {
		dev_err(dev, "can't get used_phy_port\n");
		return -EINVAL;
	}

	dev_info(phy_drd->dev, "usbphy info: version:0x%x, refclk:0x%x\n",
		phy_drd->usbphy_info.version, phy_drd->usbphy_info.refclk);

	return 0;
}

static int exynos_usbdrd_get_iptype(struct exynos_usbdrd_phy *phy_drd)
{
	struct device *dev = phy_drd->dev;
	int ret, value;

	ret = of_property_read_u32(dev->of_node, "ip_type", &value);
	if (ret) {
		dev_err(dev, "can't get ip type");
		return ret;
	}

	switch (value) {
	case TYPE_USB3DRD:
		phy_drd->ip_type = TYPE_USB3DRD;
		dev_info(dev, "It is TYPE USB3DRD");
		break;
	case TYPE_USB3HOST:
		phy_drd->ip_type = TYPE_USB3HOST;
		dev_info(dev, "It is TYPE USB3HOST");
		break;
	case TYPE_USB2DRD:
		phy_drd->ip_type = TYPE_USB2DRD;
		dev_info(dev, "It is TYPE USB2DRD");
		break;
	case TYPE_USB2HOST:
		phy_drd->ip_type = TYPE_USB2HOST;
		dev_info(dev, "It is TYPE USB2HOST");
	default:
		break;
	}

	return 0;
}

static void exynos_usbdrd_pipe3_exit(struct exynos_usbdrd_phy *phy_drd)
{
	/* pipe3 phy diable is exucuted in utmi_exit.
	 * Later divide the exit of main and sub phy if necessary
	 */
	return;
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

static void exynos_usbdrd_pipe3_init(struct exynos_usbdrd_phy *phy_drd)
{
	int value;

	if (gpio_is_valid(phy_drd->phy_port)) {
		if (phy_drd->reverse_phy_port)
			value = !gpio_get_value(phy_drd->phy_port);
		else
			value = gpio_get_value(phy_drd->phy_port);
		dev_info(phy_drd->dev, "%s: phy port[%d]\n", __func__,
						phy_drd->usbphy_info.used_phy_port);
	} else {
		dev_info(phy_drd->dev, "%s: phy port fail retry\n", __func__);
		phy_drd->phy_port =  of_get_named_gpio(phy_drd->dev->of_node,
						"phy,gpio_phy_port", 0);
		if (gpio_is_valid(phy_drd->phy_port)) {
			dev_err(phy_drd->dev, "PHY CON Selection OK\n");

			if (gpio_request(phy_drd->phy_port, "PHY_CON"))
				dev_err(phy_drd->dev, "fail to request gpio %s\n", "PHY_CON");
			else
				gpio_direction_input(phy_drd->phy_port);

			if (phy_drd->reverse_phy_port)
				value = !gpio_get_value(phy_drd->phy_port);
			else
				value = gpio_get_value(phy_drd->phy_port);
			dev_info(phy_drd->dev, "%s: phy port1[%d]\n", __func__,
							phy_drd->usbphy_info.used_phy_port);
		} else {
			dev_err(phy_drd->dev, "non-DT: PHY CON Selection\n");
		}
	}

	/* Fill USBDP Combo phy init */
	phy_exynos_usb_v3p1_g2_pma_ready(&phy_drd->usbphy_info);
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

/*
 * USB LDO control was moved to phy_conn API from OTG
 * without adding one more phy interface
 */
void exynos_usbdrd_phy_conn(struct phy *phy, int is_conn)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	if (is_conn) {
		if (phy_drd->is_conn == 0) {
			dev_info(phy_drd->dev, "USB PHY isolation clear(ON)\n");
			exynos_usbdrd_utmi_phy_isol(inst, 0, inst->pmu_mask);

			dev_info(phy_drd->dev, "USB PHY Conn Set\n");
			phy_drd->is_conn = 1;

		} else
			dev_info(phy_drd->dev, "USB PHY Conn already Setted!!\n");

	} else {
		if (phy_drd->is_conn == 1) {
			dev_info(phy_drd->dev, "USB PHY Conn Clear\n");
			phy_drd->is_conn = 0;

			dev_info(phy_drd->dev, "USB PHY isolation set(OFF)\n");
			exynos_usbdrd_utmi_phy_isol(inst, 1, inst->pmu_mask);
		} else
			dev_info(phy_drd->dev, "USB PHY Conn already cleared!!\n");
	}

	return;
}
EXPORT_SYMBOL_GPL(exynos_usbdrd_phy_conn);

void exynos_usbdrd_phy_vol_set(struct phy *phy, int voltage)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	regulator_set_voltage(phy_drd->vdd075_usb, voltage, voltage);
	dev_info(phy_drd->dev, "USB 0.85 PHY: %dmV\n", voltage);

	return;
}
EXPORT_SYMBOL_GPL(exynos_usbdrd_phy_vol_set);

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

static struct device_node *exynos_usbdrd_parse_dt(void)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "samsung,exynos-usbdrd-phy");
	if (!np) {
		pr_err("%s: failed to get the usbdrd phy device node\n",
			__func__);
		goto err;
	}
	return np;
err:
	return NULL;
}

static struct exynos_usbdrd_phy *exynos_usbdrd_get_struct(void)
{
	struct device_node *np = NULL;
	struct platform_device *pdev = NULL;
	struct device *dev;
	struct exynos_usbdrd_phy *phy_drd;

	np = exynos_usbdrd_parse_dt();
	if (np) {
		pdev = of_find_device_by_node(np);
		dev = &pdev->dev;
		of_node_put(np);
		if (pdev) {
			pr_info("%s: get the %s platform_device\n",
				__func__, pdev->name);

			phy_drd = dev->driver_data;
			return phy_drd;
		}
	}

	pr_err("%s: failed to get the platform_device\n", __func__);
	return NULL;
}

static int exynos_usbdrd_phy_power_off(struct phy *phy)
{
	return 0;
}

int exynos_usbdrd_ldo_manual_control(bool on)
{
	struct exynos_usbdrd_phy *phy_drd;

	pr_info("%s ldo = %d\n", __func__, on);

	phy_drd = exynos_usbdrd_get_struct();

	if (!phy_drd) {
		pr_err("[%s] exynos_usbdrd_get_struct error\n", __func__);
		return -ENODEV;
	}
	exynos_usbdrd_ldo_control(phy_drd, on);

	return 0;
}
EXPORT_SYMBOL_GPL(exynos_usbdrd_ldo_manual_control);

int exynos_usbdrd_ldo_external_control(bool on)
{
	struct exynos_usbdrd_phy *phy_drd;

	phy_drd = exynos_usbdrd_get_struct();

	if (!phy_drd)
		return -ENODEV;

	pr_info("%s ldo = %d\n", __func__, on);
	exynos_usbdrd_ldo_control(phy_drd, on);

	return 0;
}
EXPORT_SYMBOL_GPL(exynos_usbdrd_ldo_external_control);

int exynos_usbdrd_pipe3_enable(struct phy *phy)
{
	struct phy_usb_instance *inst = phy_get_drvdata(phy);
	struct exynos_usbdrd_phy *phy_drd = to_usbdrd_phy(inst);

	phy_exynos_usb_v3p1_g2_pma_ready(&phy_drd->usbphy_info);

	return 0;
}
EXPORT_SYMBOL_GPL(exynos_usbdrd_pipe3_enable);

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
	{
		.id		= EXYNOS_DRDPHY_PIPE3,
		.phy_isol	= exynos_usbdrd_pipe3_phy_isol,
		.phy_init	= exynos_usbdrd_pipe3_init,
		.phy_exit	= exynos_usbdrd_pipe3_exit,
		.set_refclk	= exynos_usbdrd_pipe3_set_refclk,
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
	u32 pmu_offset, pmu_offset_dp, pmu_offset_tcxo, pmu_mask, pmu_mask_tcxo;
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

	phycon_base_addr = phy_drd->reg_phy;

	ret = exynos_usbdrd_get_iptype(phy_drd);
	if (ret) {
		dev_err(dev, "%s: Failed to get ip_type\n", __func__);
		return ret;
	}

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
	ret = of_property_read_u32(dev->of_node,
		"pmu_offset_tcxobuf", &pmu_offset_tcxo);
	if (ret < 0) {
		dev_err(dev, "couldn't read pmu_offset_tcxo on %s node, error = %d\n",
						dev->of_node->name, ret);
	}
	ret = of_property_read_u32(dev->of_node,
		"pmu_mask_tcxobuf", &pmu_mask_tcxo);
	if (ret < 0) {
		dev_err(dev, "couldn't read pmu_mask_tcxo on %s node, error = %d\n",
						dev->of_node->name, ret);
		pmu_mask_tcxo = 0;
	} else
		pmu_mask_tcxo = (u32)BIT(pmu_mask_tcxo);

	pmu_mask = (u32)BIT(pmu_mask);

	dev_vdbg(dev, "Creating usbdrd_phy phy\n");
	phy_drd->phy_port =  of_get_named_gpio(dev->of_node,
					"phy,gpio_phy_port", 0);
	if (gpio_is_valid(phy_drd->phy_port)) {
		dev_err(dev, "PHY CON Selection OK\n");

		ret = gpio_request(phy_drd->phy_port, "PHY_CON");
		if (ret)
			dev_err(dev, "fail to request gpio %s:%d\n", "PHY_CON", ret);
		else
			gpio_direction_input(phy_drd->phy_port);
	}
	else
		dev_err(dev, "non-DT: PHY CON Selection\n");

	ret = of_property_read_u32(dev->of_node, "reverse_con_dir", &phy_drd->reverse_phy_port);
	dev_info(dev, "reverse_con_dir = %d\n", phy_drd->reverse_phy_port);
	if (ret < 0) {
		phy_drd->reverse_phy_port = 0;
	}

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
		phy_drd->phys[i].pmu_offset_tcxobuf = pmu_offset_tcxo;
		phy_drd->phys[i].pmu_mask_tcxobuf = pmu_mask_tcxo;
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
