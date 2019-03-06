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


/* Communicated info. from MENU in CPUIdle */
struct cpuidle_info {
	unsigned int    predicted_us;
	unsigned int    latency_req;
	int             first_idx;

	/* don't re-evaluate idle state using MOCE */
	bool            bias_forbid;
};

/*
 * Save the calculated ratio through the factor and the biased value by the ratio
 * to re-evaluate the idle state entry.
 *
 * it is dependent on the information of the cpuidle driver during initialization.
 */
struct bias_cpuidle_info {
	bool            init;
	spinlock_t      lock;

	unsigned int	*orig_target_res;	/* same with cpuidle_state(C2) struct */
	unsigned int	*orig_exit_lat;		/* same with cpuidle_state(C2) struct */

	unsigned int    *bias_target_res;
	unsigned int    *bias_exit_lat;

	unsigned int    bias_state;

	unsigned int	cur_ratio;		/* integration of following ratios */
	unsigned int	*ratios;		/* ratio per factor */
	unsigned int	*weights;		/* weights of the ratios */
};

/*
 * Save information on factors affecting idle state entry.
 * Use to calculate the required ratio when calculating the deflection value.
 *
 * initialize with exynos-moce part in device tree.
 */
struct factor_info {
	spinlock_t      lock;

	/* frequency */
	int		domain_id;
	unsigned int	nfreq_factor;
	unsigned int	*freq_factor;

	/*
	 * Add knob that affects idle state entry
	 */
};

#ifdef CONFIG_ARM64_EXYNOS_MOCE
/* Menu Gov. MOCE APIs */
extern bool exynos_moce_skip(unsigned int cpu);
extern void exynos_moce_allow(void);
extern void exynos_moce_forbid(void);
extern void exynos_moce_cpuidle_info_update(unsigned int pre_us, int lat, int idx);

/* CPUIdle MOCE APIs */
extern int exynos_moce_select(unsigned int cpu, int index);
extern unsigned int exynos_moce_calculate_target_res(int target_residency, unsigned int cpu);
#else
static inline bool exynos_moce_skip(unsigned int cpu) {return false; }
static inline void exynos_moce_allow(void) { }
static inline void exynos_moce_forbid(void) { }
static inline void exynos_moce_cpuidle_info_update(unsigned int pre_us, int lat, int idx)
{ }

static inline int exynos_moce_select(unsigned int cpu, int index) {return index; }
static inline unsigned int exynos_moce_calculate_target_res(int target_residency, unsigned int cpu)
{return target_residency; }
#endif
