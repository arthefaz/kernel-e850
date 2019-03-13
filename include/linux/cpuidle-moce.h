/*
 * Copyright (c) 2018 Hong Hyunji, Samsung Electronics Co., Ltd <hyunji.hong@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Exynos MOCE(MOCE that can Diversify Conditions for Entry into Idle State) driver implementation
 * MOCE can vary the target residency and exit latency in accordance with the frequency
 */

/*
 * Information about each factor that affects idle state entry,
 * initialized through the device tree.
 */
struct factor {
	/* list of factor */
	struct list_head	list;

	/* lock */
	spinlock_t		lock;

	/* factor type */
	int			type;

	/* factor domain */
	int			domain;

	/* weight of factor per c-state */
	unsigned int		*weight;

	/* ratio table of factor */
	unsigned int		size;
	unsigned int		ratio;
	unsigned int		*ratio_table;
};

/*
 * Information that is directly required to bias the entry conditions of the idle state,
 * and is managed by the per-cpu variable.
 */
struct bias_cpuidle {
	/* check biased */
	bool			biased;

	/*total ratio of factors per c-state */
	unsigned int		*total_ratio;

	/* head of factor list */
	struct list_head	factor_list;
};

#ifdef CONFIG_ARM64_EXYNOS_MOCE
/* CPUIdle MOCE APIs */
extern unsigned int exynos_moce_get_ratio(int state, unsigned int cpu);
#else
static inline unsigned int exynos_moce_get_ratio(int state, unsigned int cpu)
{return 1; }
#endif
