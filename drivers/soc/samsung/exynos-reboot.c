/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *	      http://www.samsung.com/
 *
 * Exynos - Support SoC specific Reboot
 * Author: Hosung Kim <hosung0.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/soc/samsung/exynos-soc.h>
#include <soc/samsung/exynos-debug.h>
#include <linux/debug-snapshot.h>
#include "../../../lib/debug-snapshot-local.h"

#ifdef CONFIG_EXYNOS_ACPM
#include <soc/samsung/acpm_ipc_ctrl.h>
#endif
#include <soc/samsung/exynos-pmu.h>

extern void (*arm_pm_restart)(enum reboot_mode reboot_mode, const char *cmd);
static void __iomem *exynos_pmu_base = NULL;

static const char * const big_cores[] = {
	"arm,mongoose",
	"arm,meerkat",
	"arm,cortex-a73",
	NULL,
};

static bool is_big_cpu(struct device_node *cn)
{
	const char * const *lc;
	for (lc = big_cores; *lc; lc++)
		if (of_device_is_compatible(cn, *lc))
			return true;
	return false;
}

int soc_has_big(void)
{
	struct device_node *cn = NULL;
	u32 big_cpu_cnt = 0;

	/* find arm,mongoose compatable in device tree */
	while ((cn = of_find_node_by_type(cn, "cpu"))) {
		if (is_big_cpu(cn))
			big_cpu_cnt++;
	}
	return big_cpu_cnt;
}

#define CPU_RESET_DISABLE_FROM_SOFTRESET	(0x041C)
#define CPU_RESET_DISABLE_FROM_WDTRESET	(0x0414)
#define BIG_CPU0_RESET			(0x220C)
#define BIG_NONCPU_ETC_RESET		(0x242C)
#define	PMU_CPU_OFFSET			(0x80)
#define SWRESET				(0x0400)
#define RESET_SEQUENCER_CONFIGURATION	(0x0500)
#define PS_HOLD_CONTROL			(0x330C)
#define EXYNOS_PMU_SYSIP_DAT0			(0x0810)

#define INFORM_NONE		0x0
#define INFORM_RAMDUMP		0xd
#define INFORM_RECOVERY		0xf

#define REBOOT_MODE_NORMAL		0x00
#define REBOOT_MODE_CHARGE		0x0A
/* Reboot into fastboot mode */
#define REBOOT_MODE_FASTBOOT		0xFC
/* Reboot into recovery */
#define REBOOT_MODE_RECOVERY		0xFF
/* Reboot into recovery */
#define REBOOT_MODE_FACTORY		0xFD

#if !defined(CONFIG_SEC_REBOOT)
#ifdef CONFIG_OF
static void exynos_power_off(void)
{
	int poweroff_try = 0;
	int power_gpio = -1;
	unsigned int keycode = 0;
	struct device_node *np, *pp;

	np = of_find_node_by_path("/gpio_keys");
	if (!np)
		return;

	for_each_child_of_node(np, pp) {
		if (!of_find_property(pp, "gpios", NULL))
			continue;
		of_property_read_u32(pp, "linux,code", &keycode);
		if (keycode == KEY_POWER) {
			pr_info("%s: <%u>\n", __func__,  keycode);
			power_gpio = of_get_gpio(pp, 0);
			break;
		}
	}

	of_node_put(np);

	if (!gpio_is_valid(power_gpio)) {
		pr_err("Couldn't find power key node\n");
		return;
	}

	while (1) {
		/* wait for power button release */
		if (gpio_get_value(power_gpio)) {
#ifdef CONFIG_EXYNOS_ACPM
			exynos_acpm_reboot();
#endif
			dbg_snapshot_scratch_reg(DSS_SIGN_RESET);
			pr_emerg("%s: Set PS_HOLD Low.\n", __func__);
			writel(readl(exynos_pmu_base + PS_HOLD_CONTROL) & 0xFFFFFEFF,
						exynos_pmu_base + PS_HOLD_CONTROL);

			++poweroff_try;
			pr_emerg("%s: Should not reach here! (poweroff_try:%d)\n",
				 __func__, poweroff_try);
		} else {
			/* if power button is not released, wait and check TA again */
			pr_info("%s: PowerButton is not released.\n", __func__);
		}
		mdelay(1000);
	}
}
#else
static void exynos_power_off(void)
{
	pr_info("Exynos power off does not support.\n");
}
#endif
#endif

static void exynos_reboot(enum reboot_mode mode, const char *cmd)
{
	u32 soc_id, revision;
	void __iomem *addr;

	if (!exynos_pmu_base)
		return;
#ifdef CONFIG_EXYNOS_ACPM
	exynos_acpm_reboot();
#endif
	printk("[%s] reboot cmd: %s\n", __func__, cmd);

	addr = exynos_pmu_base + EXYNOS_PMU_SYSIP_DAT0;
	if (cmd) {
		if (!strcmp((char *)cmd, "charge")) {
			__raw_writel(REBOOT_MODE_CHARGE, addr);
		} else if (!strcmp(cmd, "bootloader") || !strcmp(cmd, "bl") ||
				!strcmp((char *)cmd, "fastboot") || !strcmp(cmd, "fb")) {
			__raw_writel(REBOOT_MODE_FASTBOOT, addr);
		} else if (!strcmp(cmd, "recovery")) {
			__raw_writel(REBOOT_MODE_RECOVERY, addr);
		} else if (!strcmp(cmd, "sfactory")) {
			__raw_writel(REBOOT_MODE_FACTORY, addr);
		}
	}

	/* Check by each SoC */
	soc_id = exynos_soc_info.product_id;
	revision = exynos_soc_info.revision;
	pr_info("SOC ID %X. Revision: %x\n", soc_id, revision);
	switch(soc_id) {
	case EXYNOS9810_SOC_ID:
		if (revision < EXYNOS_MAIN_REV_1) {
			pr_emerg("%s: Exynos SoC reset right now with fake watchdog\n", __func__);
			s3c2410wdt_set_emergency_reset(1000, 1);

			while (1)
				wfi();
		}
		break;
	case EXYNOS9610_SOC_ID:
		break;
	default:
		break;
	}

	/* Do S/W Reset */
	pr_emerg("%s: Exynos SoC reset right now\n", __func__);
	__raw_writel(0x1, exynos_pmu_base + SWRESET);
}

static int __init exynos_reboot_setup(struct device_node *np)
{
	int err = 0;
	u32 id;

	if (!of_property_read_u32(np, "pmu_base", &id)) {
		exynos_pmu_base = ioremap(id, SZ_16K);
		if (!exynos_pmu_base) {
			pr_err("%s: failed to map to exynos-pmu-base address 0x%x\n",
				__func__, id);
			err = -ENOMEM;
		}
	}

	of_node_put(np);

	pr_info("[Exynos Reboot]: Success to register arm_pm_restart\n");

#ifndef CONFIG_DEBUG_SNAPSHOT
	/* If Debug-Snapshot is disabed, This code prevents entering fastboot */
	writel(0, exynos_pmu_base + RESET_SEQUENCER_CONFIGURATION);
#endif
	return err;
}

static const struct of_device_id reboot_of_match[] __initconst = {
	{ .compatible = "exynos,reboot", .data = exynos_reboot_setup},
	{},
};

typedef int (*reboot_initcall_t)(const struct device_node *);
static int __init exynos_reboot_init(void)
{
	struct device_node *np;
	const struct of_device_id *matched_np;
	reboot_initcall_t init_fn;

	np = of_find_matching_node_and_match(NULL, reboot_of_match, &matched_np);
	if (!np)
		return -ENODEV;

	arm_pm_restart = exynos_reboot;
#if !defined(CONFIG_SEC_REBOOT)
	pm_power_off = exynos_power_off;
#endif
	init_fn = (reboot_initcall_t)matched_np->data;

	return init_fn(np);
}
subsys_initcall(exynos_reboot_init);
