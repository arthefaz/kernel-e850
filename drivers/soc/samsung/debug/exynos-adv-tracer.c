/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/firmware.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/debug-snapshot.h>
#include <linux/debug-snapshot-soc.h>

#include <soc/samsung/exynos-adv-tracer-ipc.h>

static struct adv_tracer_info *exynos_adv_tracer;

static int adv_tracer_probe(struct platform_device *pdev)
{
	struct adv_tracer_info *adv_tracer;
	int ret = 0;

	dev_info(&pdev->dev, "[EAT+] %s\n", __func__);

	adv_tracer = devm_kzalloc(&pdev->dev,
				sizeof(struct adv_tracer_info), GFP_KERNEL);

	if (IS_ERR(adv_tracer))
		return PTR_ERR(adv_tracer);

	adv_tracer->dev = &pdev->dev;

	if (adv_tracer_ipc_init(pdev))
		goto out;

	exynos_adv_tracer = adv_tracer;

	dev_info(&pdev->dev, "[EAT-] %s\n", __func__);
out:
	return ret;
}

static int adv_tracer_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id adv_tracer_match[] = {
	{ .compatible = "samsung,exynos-adv-tracer" },
	{},
};

static struct platform_driver samsung_adv_tracer_driver = {
	.probe	= adv_tracer_probe,
	.remove	= adv_tracer_remove,
	.driver	= {
		.name = "exynos-adv-tracer",
		.owner	= THIS_MODULE,
		.of_match_table	= adv_tracer_match,
	},
};

static int __init exynos_adv_tracer_init(void)
{
	return platform_driver_register(&samsung_adv_tracer_driver);
}
arch_initcall_sync(exynos_adv_tracer_init);
