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
#include <linux/slab.h>
#include <linux/cpuidle.h>
#include <linux/cpufreq.h>
#include <linux/of_device.h>
#include <linux/cpuidle-moce.h>
#include <linux/kdev_t.h>

#include <soc/samsung/exynos-cpupm.h>

/*
 * Define the considered element as a number.
 * Used to read the ratios and weights variables.
 */
#define DEFAULT_RATIO		100
#define MIN_RATIO		10
#define FACTOR_NUM		1

typedef enum _factor_type {
	FREQ_FACTOR = 0,
	/* Add enum value for new-factor */
} FACTOR_TYPE;

DEFINE_PER_CPU(struct bias_cpuidle, bias_cpuidle);

static int size_state;
#define for_each_state(state)					\
	for ((state) = 0; (state) < size_state; (state)++)
#define for_each_state_from_start_and(state, start, mask)	\
	for ((state) = (start); (state) < size_state; (state)++, (void)mask)

/*****************************************************************************
 *                               HELPER FUNCTION                             *
 *****************************************************************************/

static struct factor *find_factor(struct bias_cpuidle *bias_idle, FACTOR_TYPE type)
{
	struct factor *pos;

	list_for_each_entry(pos, &bias_idle->factor_list, list)
		if (pos->type == type)
			return pos;

	return NULL;
}

static unsigned int find_ratio(unsigned int *table, unsigned int size, unsigned int value)
{
	int i;

	/* Searching correct value according to state of factor */
	for (i = 0 ; i < size - 1 && value >= table[i+1]; i += 2)
		;

	return table[i];
}

static void update_total_ratio(struct bias_cpuidle *bias_idle)
{
	struct factor *pos;
	int state;
	unsigned int total_ratio;

	for_each_state(state) {
		/*
		 * The total_ratio is '100' in c1(state == 0),
		 */
		if (!state)
			continue;

		total_ratio = 0;

		list_for_each_entry(pos, &bias_idle->factor_list, list) {
			total_ratio += (pos->ratio * pos->weight[state]) / 100;
		}

		/*
		 * limit the minimum ratio to prevent problems caused by excessive bias.
		 */
		if (total_ratio < 10)
			bias_idle->total_ratio[state] = MIN_RATIO;
		else
			bias_idle->total_ratio[state] = total_ratio;
	}
}

/*****************************************************************************
 *                           EXTERNAL REFERENCE APIs                         *
 *****************************************************************************/

/*
 * This function is called to bias target_residency and exit_latency values,
 * when deciding next_state in cpupm and ilde governors.
 *
 * When called for the state of a cpu-group other than the c-state,
 * (state == -1) is taken as an argument.
 */
unsigned int exynos_moce_get_ratio(int state, unsigned int cpu)
{
	struct bias_cpuidle *bias_idle = &per_cpu(bias_cpuidle, cpu);

	/* check have factor for the cpu */
	if (!bias_idle->biased)
		return DEFAULT_RATIO;

	if (state < 0)
		state = size_state - 1;

	return bias_idle->total_ratio[state];
}
EXPORT_SYMBOL(exynos_moce_get_ratio);

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
	struct bias_cpuidle *bias_idle;
	struct factor *factor;
	int cpu, target_domain;
	unsigned int cur_ratio;
	unsigned long flags;

	if (freq->flags & CPUFREQ_CONST_LOOPS)
		return NOTIFY_OK;

	if (val != CPUFREQ_POSTCHANGE)
		return NOTIFY_OK;

	bias_idle = &per_cpu(bias_cpuidle, freq->cpu);

	/* check have factor for the cpu */
	if (!bias_idle->biased)
		return NOTIFY_OK;

	factor = find_factor(bias_idle, FREQ_FACTOR);

	target_domain = factor->domain;

	spin_lock_irqsave(&factor->lock, flags);
	cur_ratio = find_ratio(factor->ratio_table, factor->size, freq->new);
	spin_unlock_irqrestore(&factor->lock, flags);

	for_each_possible_cpu(cpu) {
		bias_idle = &per_cpu(bias_cpuidle, cpu);
		factor = find_factor(bias_idle, FREQ_FACTOR);

		if (factor->domain < target_domain)
			continue;
		else if (factor->domain > target_domain)
			break;

		spin_lock_irqsave(&factor->lock, flags);
		factor->ratio = cur_ratio;
		update_total_ratio(bias_idle);
		spin_unlock_irqrestore(&factor->lock, flags);
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
		if ((index & 0x1) == 1 && tokenized_data[index-1] < 10)
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

static FACTOR_TYPE get_factor_type(const char *name)
{
	FACTOR_TYPE type = 0;

	if (!strcmp(name, "freq-factor"))
		type = FREQ_FACTOR;
	/* Add sysfs node for the new factor  */

	return type;
}

static ssize_t show_factor(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct bias_cpuidle *bias_idle;
	struct factor *factor;
	int i, cpu, index = -1;
	ssize_t size = 0;
	unsigned long flags;
	FACTOR_TYPE type = get_factor_type(attr->name);

	for_each_possible_cpu(cpu) {
		bias_idle = &per_cpu(bias_cpuidle, cpu);

		factor = find_factor(bias_idle, type);

		if (factor->domain <= index)
			continue;
		index = factor->domain;

		spin_lock_irqsave(&factor->lock, flags);
		size += sprintf(buf + size, "%u ", index);
		for (i = 0; i < factor->size; i++) {
			size += sprintf(buf + size, "%u%s", factor->ratio_table[i],
					i & 0x1 ? ":" : " ");
			pr_info("%u ", factor->ratio_table[i]);
		}
		spin_unlock_irqrestore(&factor->lock, flags);
		size += sprintf(buf + size, "\n");

	}

	return size;
}

static ssize_t store_factor(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct bias_cpuidle *bias_idle;
	struct factor *factor;
	int ntokens, cpu;
	unsigned int target_domain = NR_CPUS;
	unsigned int *new_ratio_table = NULL;
	unsigned int *old_ratio_table = NULL;
	unsigned long flags;
	FACTOR_TYPE type = get_factor_type(attr->name);

	new_ratio_table = get_tokenized_data(buf, &ntokens, &target_domain);

	if (IS_ERR(new_ratio_table))
		return PTR_RET(new_ratio_table);

	if (target_domain >= NR_CPUS) {
		pr_info("Wrong format: domain ratio freq:ratio freq:ratio ...\n");
		return 0;
	}

	for_each_possible_cpu(cpu) {
		bias_idle = &per_cpu(bias_cpuidle, cpu);

		factor = find_factor(bias_idle, type);

		if (factor->domain < target_domain)
			continue;
		else if (factor->domain > target_domain)
			break;

		spin_lock_irqsave(&factor->lock, flags);
		if (!old_ratio_table)
			old_ratio_table = factor->ratio_table;
		factor->ratio_table = new_ratio_table;
		factor->size = ntokens;
		spin_unlock_irqrestore(&factor->lock, flags);
	}
	kfree(old_ratio_table);

	return size;
}

static DEVICE_ATTR(freq_factor, S_IRUGO | S_IWUSR, show_factor, store_factor);
/* Add sysfs node for the new factor  */

static struct attribute *cpuidle_moce_attrs[] = {
	&dev_attr_freq_factor.attr,
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

static int __init init_ratio_table(struct factor *factor,
					struct device_node *factor_node)
{
	factor->size = of_property_count_u32_elems(factor_node, "ratio-table");

	factor->ratio_table = kzalloc(sizeof(unsigned int) * factor->size, GFP_KERNEL);
	if (!factor->ratio_table)
		return -1;

	of_property_read_u32_array(factor_node, "ratio-table",
					factor->ratio_table, factor->size);

	return 0;
}

static int __init add_factor_data(struct list_head *factor_list,
					struct device_node *factor_node,
					struct factor *sub_table,
					int weight)
{
	struct factor *factor;
	struct factor *sub_factor;

	/* factor list init */
	INIT_LIST_HEAD(factor_list);

	/* alloc factor */
	factor = kzalloc(sizeof(struct factor), GFP_KERNEL);
	if (!factor)
		return -1;

	/* factor domain */
	if (of_property_read_u32(factor_node, "domain", &factor->domain))
		return -1;

	/* factor id */
	if (of_property_read_u32(factor_node, "type", &factor->type))
		return -1;

	/*
	 * ratio table of factor:
	 * If there is a ratio table of the same domain previously created, points to it.
	 */
	sub_factor = &sub_table[factor->type];

	if (factor->domain == sub_factor->domain) {
		factor->size = sub_factor->size;
		factor->ratio_table = sub_factor->ratio_table;
	} else {
		init_ratio_table(factor, factor_node);
		sub_factor->domain = factor->domain;
		sub_factor->size = factor->size;
		sub_factor->ratio_table = factor->ratio_table;
	}

	if (!factor->ratio_table)
		return -1;

	/* factor weight */
	factor->weight = kzalloc(sizeof(unsigned int) * size_state, GFP_KERNEL);
	if (!factor->weight)
		return -1;

	factor->weight[0] = 0;
	factor->weight[1] = weight;

	factor->ratio = DEFAULT_RATIO;

	spin_lock_init(&factor->lock);

	list_add_tail(&factor->list, factor_list);

	of_node_put(factor_node);

	return 0;
}

static void delete_all(void)
{
	struct bias_cpuidle *bias_idle;
	struct factor *pos;
	struct factor *tmp;
	int cpu;

	for_each_possible_cpu(cpu) {
		bias_idle = &per_cpu(bias_cpuidle, cpu);
		kfree(bias_idle->total_ratio);

		list_for_each_entry_safe(pos, tmp, &bias_idle->factor_list, list) {
			kfree(pos->weight);
			kfree(pos->ratio_table);
			list_del(&pos->list);
			kfree(pos);
		}
	}
}

static int __init init_factor_list(struct bias_cpuidle *bias_idle,
					struct device_node *state_node,
					struct factor *sub_table,
					int state)
{
	struct device_node *factor_node;
	struct factor *factor = list_first_entry(&bias_idle->factor_list,
							struct factor, list);
	int i, ret = 0;
	unsigned int weight;

	for (i = 0; ; i++) {
		if (of_property_read_u32_index(state_node, "moce-weights", i, &weight))
			break;

		if (state > 1) {
			factor = container_of(factor->list.next, struct factor, list);
			factor->weight[state] = weight;
			continue;
		}

		/* init factor list only in first loop */
		factor_node = of_parse_phandle(state_node, "moce-factors", i);
		if (!factor_node) {
			pr_err("the number of factors for the state%d is too small", state);
			return -1;
		}

		ret = add_factor_data(&bias_idle->factor_list, factor_node,
							sub_table, weight);
		if (ret < 0) {
			of_node_put(factor_node);
			delete_all();
			return -1;
		}
	}
	of_node_put(state_node);

	return i;
}

static int __init dt_init_bias_idle(int cpu, struct factor *sub_table)
{
	struct device_node *cpu_node;
	struct device_node *state_node;
	struct bias_cpuidle *bias_idle;
	int i, size = 0;

	cpu_node = of_cpu_device_node_get(cpu);
	bias_idle = &per_cpu(bias_cpuidle, cpu);

	bias_idle->total_ratio = kzalloc(sizeof(unsigned int) * size_state, GFP_KERNEL);
	if (!bias_idle->total_ratio) {
		of_node_put(cpu_node);
		return -1;
	}

	/*
	 * the c1 state is not specified in the device tree,
	 * so set it separately.
	 */
	bias_idle->total_ratio[0] = DEFAULT_RATIO;

	for (i = 0; ; i++) {
		state_node = of_parse_phandle(cpu_node, "cpu-idle-states", i);
		if (!state_node)
			break;

		size = init_factor_list(bias_idle, state_node, sub_table, i + 1);
		if (size < 0) {
			of_node_put(state_node);
			of_node_put(cpu_node);
			return size;
		}

		bias_idle->total_ratio[i + 1] = DEFAULT_RATIO;
	}

	/* if (size == 0), the cpu does not have a factor */
	if (size)
		bias_idle->biased = true;

	of_node_put(cpu_node);

	return 0;
}

static int __init init_bias_idle(void)
{
	struct factor sub_table[FACTOR_NUM];
	int cpu, i, ret = 0;

	/* init sub_table */
	for (i = 0; i < FACTOR_NUM; i++)
		sub_table[i].domain = -1;

	for_each_possible_cpu(cpu) {
		ret = dt_init_bias_idle(cpu, sub_table);
		if (ret < 0) {
			pr_err("failed to initialize moce factors for cpu%d", cpu);
			break;
		}
	}

	return ret;
}

static int __init exynos_moce_init(void)
{
	int ret = 0;

	size_state = cpuidle_get_state_size();

	ret = init_bias_idle();
	if (ret < 0) {
		pr_err("failed to initialize moce driver\n");
		return ret;
	}

	init_sysfs();

	cpufreq_register_notifier(&exynos_cpufreq_trans_nb,
			CPUFREQ_TRANSITION_NOTIFIER);

	pr_info("initialized exynos moce driver\n");

	return ret;
}
device_initcall(exynos_moce_init)
