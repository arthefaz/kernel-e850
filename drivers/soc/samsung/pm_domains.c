// SPDX-License-Identifier: GPL-2.0
//
// Exynos Generic power domain support.
//
// Copyright (c) 2012 Samsung Electronics Co., Ltd.
//		http://www.samsung.com
//
// Implementation of Exynos specific power domain control which is used in
// conjunction with runtime-pm. Support for both device-tree and non-device-tree
// based power domain support is included.

#include <linux/io.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/pm_domain.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/pm_runtime.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>

#include <dt-bindings/power/samsung,exynos850-power.h>

/* Register offsets inside Power Domain area in PMU */
#define EXYNOS_PD_CONF		0x0
#define EXYNOS_PD_STATUS	0x4

struct exynos_pm_domain_config {
	/* Value for LOCAL_PWR_CFG and STATUS fields for each domain */
	u32 local_pwr_cfg;

	/* Power domain offsets in PMU area, for each power domain index */
	const unsigned int *pd_offsets;
	size_t pd_offsets_num;
};

/*
 * Exynos specific wrapper around the generic power domain
 */
struct exynos_pm_domain {
	struct device *dev;
	void __iomem *base;
	struct generic_pm_domain pd;
	u32 local_pwr_cfg;

	unsigned int offset;
	struct regmap *pmureg;
};

static void exynos_pd_write(struct exynos_pm_domain *pd, unsigned int reg,
			    unsigned int mask, unsigned int val)
{
	if (pd->pmureg) {
		regmap_update_bits(pd->pmureg, pd->offset + reg, mask, val);
	} else {
		u32 v;

		v = readl_relaxed(pd->base + reg);
		v = (v & ~mask) | val;
		writel_relaxed(v, pd->base + reg);
	}
}

static void exynos_pd_read(struct exynos_pm_domain *pd, unsigned int reg,
			   unsigned int *val)
{
	if (pd->pmureg)
		regmap_read(pd->pmureg, pd->offset + reg, val);
	else
		*val = readl_relaxed(pd->base + reg);
}

static unsigned int exynos_pd_read_status(struct exynos_pm_domain *pd)
{
	unsigned int val;

	exynos_pd_read(pd, EXYNOS_PD_STATUS, &val);
	val &= pd->local_pwr_cfg;

	return val;
}

static void exynos_pd_write_conf(struct exynos_pm_domain *pd, u32 val)
{
	exynos_pd_write(pd, EXYNOS_PD_CONF, pd->local_pwr_cfg, val);
}

static int exynos_pd_power(struct generic_pm_domain *domain, bool power_on)
{
	struct exynos_pm_domain *pd;
	u32 timeout, pwr;
	char *op;

	pd = container_of(domain, struct exynos_pm_domain, pd);

	pwr = power_on ? pd->local_pwr_cfg : 0;
	exynos_pd_write_conf(pd, pwr);

	/* Wait max 1ms */
	timeout = 10;

	while (exynos_pd_read_status(pd) != pwr) {
		if (!timeout) {
			op = (power_on) ? "enable" : "disable";
			pr_err("Power domain %s %s failed\n", domain->name, op);
			return -ETIMEDOUT;
		}
		timeout--;
		cpu_relax();
		usleep_range(80, 100);
	}

	return 0;
}

static int exynos_pd_power_on(struct generic_pm_domain *domain)
{
	return exynos_pd_power(domain, true);
}

static int exynos_pd_power_off(struct generic_pm_domain *domain)
{
	return exynos_pd_power(domain, false);
}

static const struct exynos_pm_domain_config exynos4210_cfg = {
	.local_pwr_cfg		= 0x7,
};

static const struct exynos_pm_domain_config exynos5433_cfg = {
	.local_pwr_cfg		= 0xf,
};

static const unsigned int exynos850_pd_offsets[] = {
	[EXYNOS850_PD_HSI]	= 0x1c80,
	[EXYNOS850_PD_G3D]	= 0x1d00,
	[EXYNOS850_PD_MFCMSCL]	= 0x1d80,
	[EXYNOS850_PD_DPU]	= 0x2000,
	[EXYNOS850_PD_AUD]	= 0x2080,
	[EXYNOS850_PD_IS]	= 0x2100,
};

static const struct exynos_pm_domain_config exynos850_cfg = {
	.local_pwr_cfg		= 0x1,
	.pd_offsets		= exynos850_pd_offsets,
	.pd_offsets_num		= ARRAY_SIZE(exynos850_pd_offsets),
};

static const struct of_device_id exynos_pm_domain_of_match[] = {
	{
		.compatible = "samsung,exynos4210-pd",
		.data = &exynos4210_cfg,
	}, {
		.compatible = "samsung,exynos5433-pd",
		.data = &exynos5433_cfg,
	}, {
		.compatible = "samsung,exynos850-pd",
		.data = &exynos850_cfg,
	},
	{ },
};

static int exynos_pd_parse_dt(struct exynos_pm_domain *pd)
{
	const struct exynos_pm_domain_config *variant;
	struct device *dev = pd->dev;
	struct device_node *np = dev->of_node;
	const char *name;
	u32 index;
	int ret;

	variant = of_device_get_match_data(dev);
	pd->local_pwr_cfg = variant->local_pwr_cfg;

	if (of_property_read_string(np, "label", &name) < 0)
		name = kbasename(np->full_name);
	pd->pd.name = devm_kstrdup_const(dev, name, GFP_KERNEL);
	if (!pd->pd.name)
		return -ENOMEM;

	ret = of_property_read_u32(np, "samsung,pd-index", &index);
	if (!ret) {
		if (index >= variant->pd_offsets_num)
			return -EINVAL;
		if (!dev->parent)
			return -ENODEV;

		pd->offset = variant->pd_offsets[index];
		pd->pmureg = syscon_node_to_regmap(dev->parent->of_node);
		if (IS_ERR(pd->pmureg))
			return PTR_ERR(pd->pmureg);
	} else {
		pd->base = of_iomap(np, 0);
		if (!pd->base)
			return -ENODEV;
	}

	return 0;
}

static int exynos_pd_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct of_phandle_args child, parent;
	struct exynos_pm_domain *pd;
	int on, ret;

	pd = devm_kzalloc(dev, sizeof(*pd), GFP_KERNEL);
	if (!pd)
		return -ENOMEM;

	pd->dev = dev;
	ret = exynos_pd_parse_dt(pd);
	if (ret)
		return ret;

	pd->pd.power_off = exynos_pd_power_off;
	pd->pd.power_on = exynos_pd_power_on;

	on = exynos_pd_read_status(pd);
	pm_genpd_init(&pd->pd, NULL, !on);
	ret = of_genpd_add_provider_simple(np, &pd->pd);

	if (ret == 0 && of_parse_phandle_with_args(np, "power-domains",
				      "#power-domain-cells", 0, &parent) == 0) {
		child.np = np;
		child.args_count = 0;

		if (of_genpd_add_subdomain(&parent, &child))
			pr_warn("%pOF failed to add subdomain: %pOF\n",
				parent.np, child.np);
		else
			pr_info("%pOF has as child subdomain: %pOF.\n",
				parent.np, child.np);
	}

	pm_runtime_enable(dev);
	return ret;
}

static struct platform_driver exynos_pd_driver = {
	.probe	= exynos_pd_probe,
	.driver	= {
		.name		= "exynos-pd",
		.of_match_table	= exynos_pm_domain_of_match,
		.suppress_bind_attrs = true,
	}
};

static __init int exynos4_pm_init_power_domain(void)
{
	return platform_driver_register(&exynos_pd_driver);
}
core_initcall(exynos4_pm_init_power_domain);
