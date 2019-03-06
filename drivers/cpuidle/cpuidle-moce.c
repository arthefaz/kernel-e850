/*
 * Copyright (c) 2018 Hong Hyunji, Samsung Electronics Co., Ltd <hyunji.hong@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Exynos MOCE(MOCE that can Diversify Conditions for Entry into Idle State) driver implementation
 *
 * MOCE can vary the target residency and exit latency in accordance with the frequency
 */

#include <linux/init.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/cpumask.h>
#include <linux/cpuidle.h>
#include <linux/cpufreq.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/cpuidle-moce.h>
#include <linux/kdev_t.h>

#include <soc/samsung/exynos-cpupm.h>

/*
 * Define the considered element as a number.
 * Used to read the ratios and weights variables.
 */
#define DEFAULT_RATIO		100
#define FACTOR_NUM		1

enum _factor {
	FREQ_FACTOR = 0,
} factor;

static int size_state;
#define for_each_state(state)					\
	for ((state) = 0; (state) < size_state; (state)++)
#define for_each_state_from_start_and(state, start, mask)	\
	for ((state) = (start); (state) < size_state; (state)++, (void)mask)

/* per-cpu varriables */
DEFINE_PER_CPU(struct cpuidle_info, idle_info);
DEFINE_PER_CPU(struct bias_cpuidle_info, bias_idle_info);
DEFINE_PER_CPU(struct factor_info, factor_info);

/*****************************************************************************
 *                               HELPER FUNCTION                             *
 *****************************************************************************/

static unsigned int find_ratio(unsigned int *factor_table,
		unsigned int table_size, unsigned int value)
{
	int i;

	/* Searching correct value according to state of factor */
	for (i = 0 ; i < table_size - 1 &&
			value >= factor_table[i+1]; i += 2)
		;

	return factor_table[i];
}

static void calculate_cur_ratio(struct bias_cpuidle_info *bias_info)
{
	int i;
	unsigned int total_ratio = 0;

	for (i = 0; i < FACTOR_NUM; i++)
		total_ratio += (bias_info->ratios[i] * bias_info->weights[i]) / 100;

	/* if there are more factors later, add ratio. */
	bias_info->cur_ratio =  total_ratio;
}

static unsigned int __exynos_moce_calculate_target_res(int target_residency, unsigned int ratio)
{
	return (target_residency * ratio) / 100;
}

static void update_bias_idle_info(unsigned int cpu)
{
	struct bias_cpuidle_info *bias_info;
	int state;
	unsigned int b_target_res, b_exit_lat;
	unsigned long flags;

	bias_info = &per_cpu(bias_idle_info, cpu);

	spin_lock_irqsave(&bias_info->lock, flags);
	calculate_cur_ratio(bias_info);

	for_each_state(state) {
		/*
		 * MOCE supported C2 state or deeper
		 * states[0] == C1
		 */
		if (state == 0)
			continue;

		b_target_res = __exynos_moce_calculate_target_res(
				bias_info->orig_target_res[state],
				bias_info->cur_ratio);
		b_exit_lat = (bias_info->orig_exit_lat[state] * bias_info->cur_ratio) / 100;

		bias_info->bias_target_res[state] = b_target_res;
		bias_info->bias_exit_lat[state] = b_exit_lat;
	}

	spin_unlock_irqrestore(&bias_info->lock, flags);
}

/*****************************************************************************
 *                           EXTERNAL REFERENCE APIs                         *
 *****************************************************************************/

/* Check MOCE domain init && gov.'s opinion about biasing */
bool exynos_moce_skip(unsigned int cpu)
{
	struct cpuidle_info *info = this_cpu_ptr(&idle_info);
	struct bias_cpuidle_info *bias_info = &per_cpu(bias_idle_info, cpu);

	return !bias_info->init || info->bias_forbid;
}
EXPORT_SYMBOL(exynos_moce_skip);

/* Enable MOCE per cpu */
void exynos_moce_allow(void)
{
	struct cpuidle_info *info = this_cpu_ptr(&idle_info);

	info->bias_forbid = false;
}
EXPORT_SYMBOL(exynos_moce_allow);

/* Disable MOCE per cpu */
void exynos_moce_forbid(void)
{
	struct cpuidle_info *info = this_cpu_ptr(&idle_info);

	info->bias_forbid = true;
}
EXPORT_SYMBOL(exynos_moce_forbid);

/*
 * For system-wide C state, calculate target residency with ratio per cpu.
 */
unsigned int exynos_moce_calculate_target_res(int target_residency, unsigned int cpu)
{
	struct bias_cpuidle_info *bias_info = &per_cpu(bias_idle_info, cpu);
	unsigned int ratio;
	unsigned long flags;

	if (!exynos_moce_skip(cpu))
		return target_residency;

	spin_lock_irqsave(&bias_info->lock, flags);
	ratio = bias_info->cur_ratio;
	spin_unlock_irqrestore(&bias_info->lock, flags);

	return __exynos_moce_calculate_target_res(target_residency, ratio);
}
EXPORT_SYMBOL(exynos_moce_calculate_target_res);

/*****************************************************************************
 *                           CPUIdle REFERENCE APIs                          *
 *****************************************************************************/

#define check_state_bias_enable(i, mask) ((mask) & (0x1 << (i)))

/*
 * Re-evaluate idle state entry
 */
int exynos_moce_select(unsigned int cpu, int index)
{
	struct cpuidle_info *info = this_cpu_ptr(&idle_info);
	struct bias_cpuidle_info *bias_info;
	unsigned int bias_target_residency;
	unsigned int bias_exit_latency;
	int state, next_state;

	if (exynos_moce_skip(cpu))
		return index;

	bias_info = &per_cpu(bias_idle_info, cpu);

	/* Re-evaluate the depth of idle in accordance with the variation */
	next_state = -1;
	for_each_state_from_start_and(state, info->first_idx, bias_info->bias_state) {
		/* s->disabled || su->disable */
		if (cpuidle_check_state_enable(cpu, state))
			continue;

		/*
		 * If the different biasing timing is applied onto plural idle states,
		 * bias_info also must have the different biased value for each idle state.
		 * But now the ratio of frequency factor per idle state is the same,
		 * and only the orig value is different.
		 *
		 * [For lowering the idle transition cost]
		 * Fetch the criteria for idle state without spinlock
		 * with the way of Read-Copy-No update
		 */
		spin_lock(&bias_info->lock);
		bias_target_residency = bias_info->bias_target_res[state];
		bias_exit_latency = bias_info->bias_exit_lat[state];
		spin_unlock(&bias_info->lock);

		if (bias_target_residency > info->predicted_us)
			break;
		if (bias_exit_latency > info->latency_req)
			break;

		next_state = state;
	}

	if(next_state < 0)
		return index;

	return next_state;
}
EXPORT_SYMBOL(exynos_moce_select);

void exynos_moce_cpuidle_info_update(unsigned int pre_us, int lat, int idx)
{
	struct cpuidle_info *info = this_cpu_ptr(&idle_info);

	info->predicted_us = pre_us;
	info->latency_req = lat;
	info->first_idx = idx;
}
EXPORT_SYMBOL(exynos_moce_cpuidle_info_update);

/*****************************************************************************
 *                            DEFINE NOTIFIER CALL                           *
 *****************************************************************************/

/*
 * cpufreq trans call back function :
 * search and save the ratio in the moce_domain of the cpu.
 * if the ratio was changed, calculate and save values applied ratio.
 */
static int exynos_moce_cpufreq_trans_notifier(struct notifier_block *nb,
		unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = data;
	struct bias_cpuidle_info *bias_info;
	struct factor_info *factor;
	int cpu, target_domain;
	unsigned long flags;

	if (freq->flags & CPUFREQ_CONST_LOOPS)
		return NOTIFY_OK;

	if (val != CPUFREQ_POSTCHANGE)
		return NOTIFY_OK;

	factor = &per_cpu(factor_info, freq->cpu);
	target_domain = factor->domain_id;

	for_each_possible_cpu(cpu) {
		factor = &per_cpu(factor_info, cpu);
		bias_info = &per_cpu(bias_idle_info, cpu);

		if (factor->domain_id != target_domain)
			continue;

		spin_lock_irqsave(&factor->lock, flags);
		bias_info->ratios[FREQ_FACTOR] = find_ratio(factor->freq_factor,
				factor->nfreq_factor, freq->new);
		spin_unlock_irqrestore(&factor->lock, flags);

		update_bias_idle_info(cpu);
	}
	return NOTIFY_OK;
}

static struct notifier_block exynos_cpufreq_trans_nb = {
	.notifier_call = exynos_moce_cpufreq_trans_notifier,
};

/*****************************************************************************
 *                               SYSFS INTERFACES                            *
 *****************************************************************************/

/*
 * format:
 * {DomainID} {ratio} {freq}:{ratio} {freq}:{ratio} ...:{ratio}
 * converted into array:
 * [DomainID][ratio][freq][ratio][freq]...[ratio]
 */
static unsigned int *get_tokenized_data(const char *buf, int *num_tokens, unsigned int *domainId)
{
	const char *bbuf;
	int index;
	int ntokens = 1;
	unsigned int *tokenized_data;
	int err = -EINVAL;

	bbuf = buf;
	while ((bbuf = strpbrk(bbuf + 1, " :")))
		ntokens++;

	if ((ntokens & 0x1))
		goto err;

	tokenized_data = kmalloc((ntokens - 1) * sizeof(unsigned int), GFP_KERNEL);
	if (!tokenized_data) {
		err = -ENOMEM;
		goto err;
	}

	bbuf = buf;
	index = 0;

	if (sscanf(bbuf, "%u", domainId) != 1)
		goto err_kfree;

	if (*domainId > 2)
		goto err_kfree;

	bbuf = strpbrk(bbuf, " ");
	if (!bbuf)
		goto err_kfree;
	bbuf++;

	while (index < (ntokens - 1)) {
		if (sscanf(bbuf, "%u", &tokenized_data[index++]) != 1)
			goto err_kfree;

		/* Any percentage value except */
		if ((index & 0x1) == 1 && tokenized_data[index-1] < 0)
			goto err_kfree;
		bbuf = strpbrk(bbuf, " :");
		if (!bbuf)
			break;
		bbuf++;
	}

	if (index != (ntokens - 1))
		goto err_kfree;

	*num_tokens = ntokens - 1;
	return tokenized_data;

err_kfree:
	kfree(tokenized_data);
err:
	return ERR_PTR(err);
}

static ssize_t show_freq_factor(struct kobject *kobj,
				struct attribute *attr, char *buf)
{
	struct factor_info *factor;
	int i, cpu, index = -1;
	ssize_t size = 0;
	unsigned long flags;

	for_each_possible_cpu(cpu) {
		factor = &per_cpu(factor_info, cpu);

		if (factor->domain_id <= index)
			continue;
		index = factor->domain_id;

		spin_lock_irqsave(&factor->lock, flags);
		size += sprintf(buf + size, "%u ", index);
		for (i = 0; i < factor->nfreq_factor; i++) {
			size += sprintf(buf + size, "%u%s", factor->freq_factor[i],
					i & 0x1 ? ":" : " ");
			pr_info("%u ", factor->freq_factor[i]);
		}
		spin_unlock_irqrestore(&factor->lock, flags);
		size += sprintf(buf + size, "\n");
	}

	return size;
}

static ssize_t store_freq_factor(struct kobject *kobj, struct attribute *attr,
					const char *buf, size_t size)
{
	struct factor_info *factor;
	int ntokens, cpu;
	unsigned int target_domain = NR_CPUS;
	unsigned int *new_freq_factor = NULL;
	unsigned int *old_freq_factor = NULL;
	unsigned long flags;

	new_freq_factor = get_tokenized_data(buf, &ntokens, &target_domain);

	if (IS_ERR(new_freq_factor))
		return PTR_RET(new_freq_factor);

	if (target_domain >= NR_CPUS) {
		pr_info("Wrong format: domain_id ratio freq:ratio freq:ratio ...\n");
		return 0;
	}

	for_each_possible_cpu(cpu) {
		factor = &per_cpu(factor_info, cpu);

		if (factor->domain_id < target_domain)
			continue;
		else if (factor->domain_id > target_domain)
			break;

		spin_lock_irqsave(&factor->lock, flags);
		if (!old_freq_factor) {
			old_freq_factor = factor->freq_factor;
		}
		factor->freq_factor = new_freq_factor;
		factor->nfreq_factor = ntokens;
		spin_unlock_irqrestore(&factor->lock, flags);
	}
	kfree(old_freq_factor);

	return size;
}

static struct global_attr freq_factor =
__ATTR(freq_factor, S_IRUGO | S_IWUSR,
		show_freq_factor, store_freq_factor);

static struct attribute *cpuidle_moce_attrs[] = {
	&freq_factor.attr,
	NULL,
};

static const struct attribute_group cpuidle_moce_group = {
	.attrs = cpuidle_moce_attrs,
};

/*****************************************************************************
 *                       INITIALIZE EXYNOS MOCE DRIVER                       *
 *****************************************************************************/

extern struct class *idle_class;

static __init void init_sysfs(void)
{
	struct device *dev;

	dev = device_create(idle_class, NULL, MKDEV(0, 1), NULL, "moce_factor");

	if (sysfs_create_group(&dev->kobj, &cpuidle_moce_group))
		pr_err("%s: failed to create sysfs group", __func__);
}

static int __init init_ratios(void)
{
	struct bias_cpuidle_info *bias_info;
	int i, cpu;

	for_each_possible_cpu(cpu) {
		bias_info = &per_cpu(bias_idle_info, cpu);

		bias_info->ratios = kzalloc(sizeof(unsigned int) * FACTOR_NUM, GFP_KERNEL);
		bias_info->weights = kzalloc(sizeof(unsigned int) * FACTOR_NUM, GFP_KERNEL);
		if (!bias_info->ratios || !bias_info->weights)
			return -1;

		for (i = 0; i < FACTOR_NUM; i++) {
			bias_info->ratios[i] = 100;
			bias_info->weights[i] = 100 / FACTOR_NUM;
		}
		calculate_cur_ratio(bias_info);
	}

	return 0;
}

static void delete_ratios(void)
{
	struct bias_cpuidle_info *bias_info;
	int cpu;

	for_each_possible_cpu(cpu) {
		bias_info = &per_cpu(bias_idle_info, cpu);
		kfree(bias_info->ratios);
		kfree(bias_info->weights);
	}
}

/*
 * init bias_cpuidle_info structure
 * according cpudile state info. (target_residency, exit_latency)
 */
static int __init init_idle_state_info(void)
{
	struct bias_cpuidle_info *bias_info;
	int cpu;
	unsigned int i;

	size_state = cpuidle_get_state_size();

	for_each_possible_cpu(cpu) {
		bias_info = &per_cpu(bias_idle_info, cpu);
		bias_info->orig_target_res = kzalloc(sizeof(unsigned int) * size_state, GFP_KERNEL);
		bias_info->orig_exit_lat = kzalloc(sizeof(unsigned int) * size_state, GFP_KERNEL);
		bias_info->bias_target_res = kzalloc(sizeof(unsigned int) * size_state, GFP_KERNEL);
		bias_info->bias_exit_lat = kzalloc(sizeof(unsigned int) * size_state, GFP_KERNEL);
		if (!bias_info->orig_target_res || !bias_info->orig_exit_lat
				|| !bias_info->bias_target_res || !bias_info->bias_exit_lat) {
			pr_info("failed to allocate idle info table\n");
			return -1;
		}

		for (i = 0; i < size_state; i++) {
		/*
		 * MOCE supported C2 state or deeper
		 * states[0] == C1
		 */
			if (i == 0)
				continue;
			bias_info->bias_state |= (1 << i);
			bias_info->orig_target_res[i] = cpuidle_get_target_residency(cpu, i);
			bias_info->orig_exit_lat[i] = cpuidle_get_exit_latency(cpu, i);
			bias_info->bias_target_res[i] = bias_info->orig_target_res[i];
			bias_info->bias_exit_lat[i] = bias_info->orig_exit_lat[i];
		}
		spin_lock_init(&bias_info->lock);
		bias_info->init = true;
	}

	pr_info("Updated Exynos MOCE driver : to %d c-state\n", size_state - 1);
	return 0;
}

static const struct of_device_id of_exynos_moce_match[] = {
	{ .compatible = "samsung,exynos-moce", },
	{ },
};

static void delete_factor(void)
{
	struct factor_info *factor;
	int cpu;

	for_each_possible_cpu(cpu) {
		factor = &per_cpu(factor_info, cpu);
		kfree(factor->freq_factor);
	}
}

static int __init init_freq_data(struct factor_info *factor, struct device_node *dn)
{
	int size;
	const struct of_device_id *match_id;

	match_id = of_match_node(of_exynos_moce_match, dn);
	if (!match_id)
		return -ENODEV;

	of_property_read_u32(dn, "domain-id", &factor->domain_id);

	size = of_property_count_u32_elems(dn, "factors");
	if (size < 0) {
		pr_info("table size is '0' of frequency factor\n");
		return -1;
	}
	factor->nfreq_factor = size;

	factor->freq_factor = kzalloc(sizeof(unsigned int)
			* (factor->nfreq_factor), GFP_KERNEL);
	if (!factor->freq_factor) {
		delete_factor();
		return -1;
	}

	/* init frequency factor table */
	of_property_read_u32_array(dn, "factors", factor->freq_factor, size);

	spin_lock_init(&factor->lock);

	return 0;
}

static int __init dt_init_freq_factor(void)
{
	struct device_node *cpu_node;
	struct device_node *freq_node;
	struct factor_info *factor;
	int cpu, ret = 0;

	for_each_possible_cpu(cpu) {
		cpu_node = of_cpu_device_node_get(cpu);

		freq_node = of_parse_phandle(cpu_node, "moce-freq", 0);
		if (!freq_node)
			break;

		if (!of_device_is_available(freq_node)) {
			of_node_put(freq_node);
			continue;
		}

		factor = &per_cpu(factor_info, cpu);
		ret = init_freq_data(factor, freq_node);
		if (ret) {
			pr_err("failed to parsing freq factor for MOCE");
			break;
		}

		of_node_put(freq_node);
	}

	of_node_put(freq_node);
	of_node_put(cpu_node);

	return ret;
}

static int __init exynos_moce_init(void)
{
	int ret;

	ret = init_ratios();
	if (ret < 0) {
		pr_info("failed to allocate and initialize value for ratios\n");
		delete_ratios();
		return -1;
	}

	ret = dt_init_freq_factor();
	if (ret < 0) {
		pr_err("failed to initialize factor with device tree\n");
		return -1;
	}

	init_idle_state_info();

	init_sysfs();

	cpufreq_register_notifier(&exynos_cpufreq_trans_nb,
			CPUFREQ_TRANSITION_NOTIFIER);

	pr_info("Initialized Exynos MOCE driver\n");

	return ret;
}
device_initcall(exynos_moce_init)
