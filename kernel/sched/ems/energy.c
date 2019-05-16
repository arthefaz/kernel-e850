/*
 * Energy efficient cpu selection
 *
 * Copyright (C) 2018 Samsung Electronics Co., Ltd
 * Park Bumgyu <bumgyu.park@samsung.com>
 */

#include <trace/events/ems.h>

#include "../sched.h"
#include "ems.h"

/*
 * The compute capacity, power consumption at this compute capacity and
 * frequency of state. The cap and power are used to find the energy
 * efficiency cpu, and the frequency is used to create the capacity table.
 */
struct energy_state {
	unsigned long frequency;
	unsigned long cap;
	unsigned long power;

	/* for sse */
	unsigned long cap_s;
	unsigned long power_s;

	unsigned long static_power;
};

/*
 * Each cpu can have its own mips, coefficient and energy table. Generally,
 * cpus in the same frequency domain have the same mips, coefficient and
 * energy table.
 */
struct energy_table {
	unsigned int mips;
	unsigned int coefficient;
	unsigned int mips_s;
	unsigned int coefficient_s;
	unsigned int static_coefficient;

	struct energy_state *states;

	unsigned int nr_states;
};
static DEFINE_PER_CPU(struct energy_table, energy_table);

/* capacity energy weight */
struct ce_weight {
	unsigned int c_weight;
	unsigned int e_weight;
	unsigned int c_weight_perf;
	unsigned int e_weight_perf;
};
static DEFINE_PER_CPU(struct ce_weight, weight);

static inline unsigned int
__compute_energy(unsigned long util, unsigned long cap,
				unsigned long dp, unsigned long sp)
{
	return (dp * ((util << SCHED_CAPACITY_SHIFT) / cap))
				+ (sp << SCHED_CAPACITY_SHIFT);
}

static unsigned int
compute_energy(struct energy_table *table, struct task_struct *p,
			int target_cpu, int cap_idx)
{
	unsigned int energy;

	energy = __compute_energy(__ml_cpu_util_with(target_cpu, p, SSE),
				table->states[cap_idx].cap_s,
				table->states[cap_idx].power_s,
				table->states[cap_idx].static_power);
	energy += __compute_energy(__ml_cpu_util_with(target_cpu, p, USS),
				table->states[cap_idx].cap,
				table->states[cap_idx].power,
				table->states[cap_idx].static_power);

	return energy;
}

static unsigned int
compute_efficiency(struct task_struct *p, int target_cpu,
			unsigned int c_weight, unsigned int e_weight)
{
	struct energy_table *table = &per_cpu(energy_table, target_cpu);
	unsigned long max_util = 0;
	unsigned long capacity, energy;
	unsigned int eff;
	unsigned int cap_idx;
	int cpu, i;

	/* energy table does not exist */
	if (!table->nr_states)
		return 0;

	/* Get utilization of cpu in coregroup with task */
	for_each_cpu(cpu, cpu_coregroup_mask(target_cpu)) {
		unsigned long util;

		if (cpu == target_cpu) {
			util = ml_cpu_util_with(cpu, p);

			/* if it is over-capacity, give up eifficiency calculatation */
			if (util > table->states[table->nr_states - 1].cap)
				return 0;
		} else
			util = ml_cpu_util_without(cpu, p);

		/*
		 * The cpu in the coregroup has same capacity and the
		 * capacity depends on the cpu with biggest utilization.
		 * Find biggest utilization in the coregroup to know
		 * what capacity the cpu will have.
		 */
		if (util > max_util)
			max_util = util;
	}

	/* Find the capacity index according to biggest utilization in coregroup. */
	cap_idx = table->nr_states - 1;
	max_util = max_util + (max_util >> 2);
	for (i = 0; i < table->nr_states; i++) {
		if (table->states[i].cap >= max_util) {
			cap_idx = i;
			break;
		}
	}

	if (p->sse)
		capacity = table->states[cap_idx].cap_s;
	else
		capacity = table->states[cap_idx].cap;

	/*
	 * Compute performance efficiency
	 *  efficiency = capacity / energy
	 */
	capacity = (capacity * c_weight) << (SCHED_CAPACITY_SHIFT * 2);
	energy = compute_energy(table, p, target_cpu, cap_idx) * e_weight;
	eff = capacity / energy;

	trace_ems_compute_eff(p, target_cpu, ml_cpu_util_with(target_cpu, p),
			c_weight, e_weight, capacity, energy, eff);

	return eff;
}

static int find_best_eff(struct enrg_env *env, unsigned int *eff, int idle)
{
	unsigned int best_eff = 0;
	int best_cpu = -1;
	int cpu;
	struct cpumask candidates;

	if (idle)
		cpumask_copy(&candidates, &env->idle_candidates);
	else
		cpumask_copy(&candidates, &env->candidates);

	if (cpumask_empty(&candidates))
		return -1;

	/* find best efficiency cpu */
	for_each_cpu(cpu, &candidates) {
		unsigned int eff;

		eff = compute_efficiency(env->p, cpu,
					env->c_weight[cpu],
					env->e_weight[cpu]);
		if (eff > best_eff) {
			best_eff = eff;
			best_cpu = cpu;
		}
	}

	*eff = best_eff;

	trace_ems_best_eff(env->p, *(unsigned int *)cpumask_bits(&candidates),
						idle, best_cpu, best_eff);

	return best_cpu;
}

static void get_ready_env(struct enrg_env *env)
{
	int cpu;

	cpumask_copy(&env->candidates, &env->fit_cpus);
	cpumask_clear(&env->idle_candidates);

	for_each_cpu(cpu, &env->fit_cpus) {
		/*
		 * The weight is separated into the value for normal mode and
		 * the value for performance mode.
		 */
		if (env->prefer_perf) {
			env->c_weight[cpu] = per_cpu(weight, cpu).c_weight_perf;
			env->e_weight[cpu] = per_cpu(weight, cpu).e_weight_perf;
		} else {
			env->c_weight[cpu] = per_cpu(weight, cpu).c_weight;
			env->e_weight[cpu] = per_cpu(weight, cpu).e_weight;
		}

		/*
		 * Basically, all active cpus are candidates. But if
		 * env->prefer_idle is set to '1', the idle cpus are included in
		 * env->idle_candidates and * the running cpus are included in
		 * the env->candidate. Both candidates are exclusive.
		 */
		if (env->prefer_idle && idle_cpu(cpu)) {
			cpumask_set_cpu(cpu, &env->idle_candidates);
			cpumask_clear_cpu(cpu, &env->candidates);
		}
	}

	trace_ems_candidates(env->p,
		*(unsigned int *)cpumask_bits(&env->candidates),
		*(unsigned int *)cpumask_bits(&env->idle_candidates));
}

int find_best_cpu(struct enrg_env *env)
{
	unsigned int best_eff = 0;
	int best_cpu;
	bool best_idle = false;

	/*
	 * Get ready to find best cpu.
	 * Depending on the state of the task, the candidate cpus and C/E
	 * weight are decided.
	 */
	get_ready_env(env);

	/*
	 * Find best cpu among the running cpu and idle cpu.
	 * The best idle cpu  is meaningful only if task.prefer_idle
	 * is set to '1'. If prefer idle is not set, best_idle_cpu
	 * has a negative number and it is ignored.
	 */
	best_cpu = find_best_eff(env, &best_eff, 0);
	if (env->prefer_idle) {
		unsigned int best_idle_eff = 0;
		int best_idle_cpu;

		best_idle_cpu = find_best_eff(env, &best_idle_eff, 1);
		if (best_idle_cpu < 0)
			goto out;

		/*
		 * Multiply best idle efficiency by 1.25 to increase the idle cpu
		 * selection probability.
		 */
		best_idle_eff = best_idle_eff + (best_idle_eff >> 2);
		if (best_idle_eff >= best_eff) {
			best_eff = best_idle_eff;
			best_cpu = best_idle_cpu;
			best_idle = true;
		}
	}

out:
	trace_ems_best_cpu(env->p, best_cpu, best_eff, best_idle);

	return best_cpu;
}

/*
 * returns allowed capacity base on the allowed power
 * freq: base frequency to find base_power
 * power: allowed_power = base_power + power
 */
int find_allowed_capacity(int cpu, unsigned int freq, int power)
{
	struct energy_table *table = &per_cpu(energy_table, cpu);
	unsigned long new_power = 0;
	int i;

	/* find power budget for new frequency */
	for (i = 0; i < table->nr_states; i++)
		if (table->states[i].frequency == freq)
			break;

	/* calaculate new power budget */
	new_power = table->states[i].power + power;

	/* find minimum freq over the new power budget */
	for (i = 0; i < table->nr_states; i++)
		if (table->states[i].power >= new_power)
			return table->states[i].cap;

	/* returne the max capaity */
	return table->states[table->nr_states - 1].cap;
}

int find_step_power(int cpu, int step)
{
	struct energy_table *table = &per_cpu(energy_table, cpu);
	int max_idx = table->nr_states - 1;

	if (!step)
		return 0;

	return (table->states[max_idx].power - table->states[0].power) / step;
}

static ssize_t show_ce_weight(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int cpu, len = 0;

	for_each_possible_cpu(cpu) {
		len += sprintf(buf + len,
			"[cpu%d] (normal) C:E = %d:%d, (performance) C:E %d:%d\n",
			cpu,
			per_cpu(weight, cpu).c_weight,
			per_cpu(weight, cpu).e_weight,
			per_cpu(weight, cpu).c_weight_perf,
			per_cpu(weight, cpu).e_weight_perf);
	}

	return len;
}

static ssize_t store_ce_weight(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf,
		size_t count)
{
	int cpu, c_weight, e_weight, c_weight_perf, e_weight_perf;

	if (sscanf(buf, "%d %d %d %d %d",
			&cpu, &c_weight, &e_weight,
			&c_weight_perf, &e_weight_perf) != 5)
		return -EINVAL;

	if (!cpumask_test_cpu(cpu, cpu_possible_mask))
		return -EINVAL;

	if (c_weight < 0 || e_weight < 0 ||
			c_weight_perf < 0 || e_weight_perf < 0)
		return -EINVAL;

	per_cpu(weight, cpu).c_weight = c_weight;
	per_cpu(weight, cpu).e_weight = e_weight;
	per_cpu(weight, cpu).c_weight_perf = c_weight_perf;
	per_cpu(weight, cpu).e_weight_perf = e_weight_perf;

	return count;
}

static struct kobj_attribute ce_weight_attr =
__ATTR(ce_weight, 0644, show_ce_weight, store_ce_weight);

static int __init init_ce_weight(void)
{
	int ret;

	ret = sysfs_create_file(ems_kobj, &ce_weight_attr.attr);
	if (ret)
		pr_err("%s: faile to create sysfs file\n", __func__);

	return 0;
}
late_initcall(init_ce_weight);

/*
 * Information of per_cpu cpu capacity variable
 *
 * cpu_capacity_orig{_s}
 * : Original capacity of cpu. It never be changed since initialization.
 *
 * cpu_capacity{_s}
 * : Capacity of cpu. It is same as cpu_capacity_orig normally but it can be
 *   changed by CPU frequency restriction.
 *
 * cpu_capacity_cpufreq{_s}
 * : Capacity of cpu restricted by CPUFreq.
 *
 * cpu_capacity_qos{_s}
 * : Capacity of cpu restricted by PM QoS.
 *
 * cpu_capacity_ratio{_s}
 * : Ratio between capacity of sse and uss. It is used for calculating
 *   cpu utilization in Multi Load for optimization.
 */
static DEFINE_PER_CPU(unsigned long, cpu_capacity_orig) = SCHED_CAPACITY_SCALE;
static DEFINE_PER_CPU(unsigned long, cpu_capacity_orig_s) = SCHED_CAPACITY_SCALE;

static DEFINE_PER_CPU(unsigned long, cpu_capacity) = SCHED_CAPACITY_SCALE;
static DEFINE_PER_CPU(unsigned long, cpu_capacity_s) = SCHED_CAPACITY_SCALE;

static DEFINE_PER_CPU(unsigned long, cpu_capacity_freq) = SCHED_CAPACITY_SCALE;
static DEFINE_PER_CPU(unsigned long, cpu_capacity_freq_s) = SCHED_CAPACITY_SCALE;

static DEFINE_PER_CPU(unsigned long, cpu_capacity_qos) = SCHED_CAPACITY_SCALE;
static DEFINE_PER_CPU(unsigned long, cpu_capacity_qos_s) = SCHED_CAPACITY_SCALE;

static DEFINE_PER_CPU(unsigned long, cpu_capacity_ratio) = SCHED_CAPACITY_SCALE;
static DEFINE_PER_CPU(unsigned long, cpu_capacity_ratio_s) = SCHED_CAPACITY_SCALE;

unsigned long capacity_cpu_orig(int cpu, int sse)
{
	return sse ? per_cpu(cpu_capacity_orig_s, cpu) :
			per_cpu(cpu_capacity_orig, cpu);
}

unsigned long capacity_cpu(int cpu, int sse)
{
	return sse ? per_cpu(cpu_capacity_s, cpu) : per_cpu(cpu_capacity, cpu);
}

unsigned long capacity_ratio(int cpu, int sse)
{
	return sse ? per_cpu(cpu_capacity_ratio_s, cpu) : per_cpu(cpu_capacity_ratio, cpu);
}

static inline void __update_capacity(int cpu)
{
	unsigned long capacity, ratio;

	/* update USS capacity */
	capacity = min(per_cpu(cpu_capacity_freq, cpu),
		       per_cpu(cpu_capacity_qos, cpu));
	per_cpu(cpu_capacity, cpu) = capacity;
	topology_set_cpu_scale(cpu, per_cpu(cpu_capacity, cpu));

	/* update SSE capacity */
	capacity = min(per_cpu(cpu_capacity_freq_s, cpu),
		       per_cpu(cpu_capacity_qos_s, cpu));
	per_cpu(cpu_capacity_s, cpu) = capacity;

	/* update capacity ratio */
	ratio = (per_cpu(cpu_capacity, cpu) << SCHED_CAPACITY_SHIFT);
	ratio /= per_cpu(cpu_capacity_s, cpu);
	per_cpu(cpu_capacity_ratio, cpu) = ratio;

	ratio = (per_cpu(cpu_capacity_s, cpu) << SCHED_CAPACITY_SHIFT);
	ratio /= per_cpu(cpu_capacity, cpu);
	per_cpu(cpu_capacity_ratio_s, cpu) = ratio;
}

#define RESTRICT_CAPACITY_CPUFREQ	1
#define RESTRICT_CAPACITY_PMQOS		2

#define scale_capacity(cap, max)	((cap * max) >> SCHED_CAPACITY_SHIFT)
static void
update_capacity(struct cpumask *mask,
			unsigned long clipped_freq,
			unsigned long max_freq,
			int type)
{
	unsigned long max_scale;
	int cpu;

	max_scale = (clipped_freq << SCHED_CAPACITY_SHIFT);
	max_scale /= max_freq;

	for_each_cpu(cpu, mask) {
		unsigned long new_capacity;
		unsigned long *capacity, *capacity_s;

		if (type == RESTRICT_CAPACITY_CPUFREQ) {
			capacity = &per_cpu(cpu_capacity_freq, cpu);
			capacity_s = &per_cpu(cpu_capacity_freq_s, cpu);
		} else if (type == RESTRICT_CAPACITY_PMQOS) {
			capacity = &per_cpu(cpu_capacity_qos, cpu);
			capacity_s = &per_cpu(cpu_capacity_qos_s, cpu);
		} else
			break;

		new_capacity = per_cpu(cpu_capacity_orig, cpu) * max_scale;
		new_capacity >>= SCHED_CAPACITY_SHIFT;
		*capacity = new_capacity;

		new_capacity = per_cpu(cpu_capacity_orig_s, cpu) * max_scale;
		new_capacity >>= SCHED_CAPACITY_SHIFT;
		*capacity_s = new_capacity;

		__update_capacity(cpu);
	}
}

static void
update_capacity_freq(struct cpumask *cpus,
			unsigned long clipped_freq,
			unsigned long max_freq)
{
	update_capacity(cpus, clipped_freq, max_freq, RESTRICT_CAPACITY_CPUFREQ);
}

void
update_capacity_qos(struct cpumask *cpus,
			unsigned long clipped_freq,
			unsigned long max_freq)
{
	update_capacity(cpus, clipped_freq, max_freq, RESTRICT_CAPACITY_PMQOS);
}

static int sched_cpufreq_policy_callback(struct notifier_block *nb,
					unsigned long event, void *data)
{
	struct cpufreq_policy *policy = data;

	if (event != CPUFREQ_NOTIFY)
		return NOTIFY_DONE;

	/*
	 * When policy->max is pressed, the performance of the cpu is restricted.
	 * In the restricted state, the cpu capacity also changes, and the
	 * overutil condition changes accordingly, so the cpu capcacity is updated
	 * whenever policy is changed.
	 */
	update_capacity_freq(policy->related_cpus,
		policy->max, policy->cpuinfo.max_freq);

	return NOTIFY_OK;
}

static struct notifier_block sched_cpufreq_policy_notifier = {
	.notifier_call = sched_cpufreq_policy_callback,
};

static void
fill_power_table(struct energy_table *table, int table_size,
			unsigned long *f_table, unsigned int *v_table,
			int max_f, int min_f)
{
	int i, index = 0;
	int c = table->coefficient, c_s = table->coefficient_s, v;
	int static_c = table->static_coefficient;
	unsigned long f, power, power_s, static_power;

	/* energy table and frequency table are inverted */
	for (i = table_size - 1; i >= 0; i--) {
		if (f_table[i] > max_f || f_table[i] < min_f)
			continue;

		f = f_table[i] / 1000;	/* KHz -> MHz */
		v = v_table[i] / 1000;	/* uV -> mV */

		/*
		 * power = coefficent * frequency * voltage^2
		 */
		power = c * f * v * v;
		power_s = c_s * f * v * v;
		static_power = static_c * v * v;

		/*
		 * Generally, frequency is more than treble figures in MHz and
		 * voltage is also more then treble figures in mV, so the
		 * calculated power is larger than 10^9. For convenience of
		 * calculation, divide the value by 10^9.
		 */
		do_div(power, 1000000000);
		do_div(power_s, 1000000000);
		do_div(static_power, 1000000);
		table->states[index].power = power;
		table->states[index].power_s = power_s;
		table->states[index].static_power = static_power;

		/* save frequency to energy table */
		table->states[index].frequency = f_table[i];
		index++;
	}
}

static void
fill_cap_table(struct energy_table *table, int max_mips, unsigned long max_mips_freq)
{
	int i, m = table->mips, m_s = table->mips_s;
	unsigned long f;

	for (i = 0; i < table->nr_states; i++) {
		f = table->states[i].frequency;

		/*
		 * capacity = freq/max_freq * mips/max_mips * 1024
		 */
		table->states[i].cap = f * m * 1024 / max_mips_freq / max_mips;
		table->states[i].cap_s = f * m_s * 1024 / max_mips_freq / max_mips;
	}
}

static void show_energy_table(struct energy_table *table, int cpu)
{
	int i;

	pr_info("[Energy Table: cpu%d]\n", cpu);
	for (i = 0; i < table->nr_states; i++) {
		pr_info("[%2d] cap=%4lu power=%4lu | cap(S)=%4lu power(S)=%4lu | static-power=%4lu\n",
			i, table->states[i].cap, table->states[i].power,
			table->states[i].cap_s, table->states[i].power_s,
			table->states[i].static_power);
	}
}

/*
 * Whenever frequency domain is registered, and energy table corresponding to
 * the domain is created. Because cpu in the same frequency domain has the same
 * energy table. Capacity is calculated based on the max frequency of the fastest
 * cpu, so once the frequency domain of the faster cpu is regsitered, capacity
 * is recomputed.
 */
void init_sched_energy_table(struct cpumask *cpus, int table_size,
				unsigned long *f_table, unsigned int *v_table,
				int max_f, int min_f)
{
	struct energy_table *table;
	int cpu, i, mips, valid_table_size = 0;
	int max_mips = 0;
	unsigned long max_mips_freq = 0;
	int last_state;

	cpumask_and(cpus, cpus, cpu_possible_mask);
	if (cpumask_empty(cpus))
		return;

	mips = per_cpu(energy_table, cpumask_any(cpus)).mips;
	for_each_cpu(cpu, cpus) {
		/*
		 * All cpus in a frequency domain must have the same capacity
		 * because cpu and frequency domain always are same.
		 * Verifying domain is enough to check the mips so it does not
		 * need to check mips_s.
		 */
		if (mips != per_cpu(energy_table, cpu).mips) {
			pr_warn("cpu%d has different cpacity!!\n", cpu);
			return;
		}
	}

	/* get size of valid frequency table to allocate energy table */
	for (i = 0; i < table_size; i++) {
		if (f_table[i] > max_f || f_table[i] < min_f)
			continue;

		valid_table_size++;
	}

	/* there is no valid row in the table, energy table is not created */
	if (!valid_table_size)
		return;

	/* allocate memory for energy table and fill power table */
	for_each_cpu(cpu, cpus) {
		table = &per_cpu(energy_table, cpu);
		table->states = kcalloc(valid_table_size,
				sizeof(struct energy_state), GFP_KERNEL);
		if (unlikely(!table->states))
			return;

		table->nr_states = valid_table_size;
		fill_power_table(table, table_size, f_table, v_table, max_f, min_f);
	}

	/*
	 * Find fastest cpu among the cpu to which the energy table is allocated.
	 * The mips and max frequency of fastest cpu are needed to calculate
	 * capacity.
	 */
	for_each_possible_cpu(cpu) {
		table = &per_cpu(energy_table, cpu);
		if (!table->states)
			continue;

		mips = max(table->mips, table->mips_s);
		if (mips > max_mips) {
			max_mips = mips;

			last_state = table->nr_states - 1;
			max_mips_freq = table->states[last_state].frequency;
		}
	}

	/*
	 * Calculate and fill capacity table.
	 * Recalculate the capacity whenever frequency domain changes because
	 * the fastest cpu may have changed and the capacity needs to be
	 * recalculated.
	 */
	for_each_possible_cpu(cpu) {
		struct sched_domain *sd;

		table = &per_cpu(energy_table, cpu);
		if (!table->states)
			continue;

		fill_cap_table(table, max_mips, max_mips_freq);
		show_energy_table(table, cpu);

		last_state = table->nr_states - 1;
		per_cpu(cpu_capacity_orig_s, cpu) = table->states[last_state].cap_s;
		per_cpu(cpu_capacity_orig, cpu) = table->states[last_state].cap;
		per_cpu(cpu_capacity_s, cpu) = table->states[last_state].cap_s;
		per_cpu(cpu_capacity, cpu) = table->states[last_state].cap;
		topology_set_cpu_scale(cpu, table->states[last_state].cap);

		rcu_read_lock();
		for_each_domain(cpu, sd)
			update_group_capacity(sd, cpu);
		rcu_read_unlock();
	}

	topology_update();
}

static int __init init_sched_energy_data(void)
{
	struct device_node *cpu_node, *cpu_phandle;
	int cpu;

	for_each_possible_cpu(cpu) {
		struct energy_table *table;

		cpu_node = of_get_cpu_node(cpu, NULL);
		if (!cpu_node) {
			pr_warn("CPU device node missing for CPU %d\n", cpu);
			return -ENODATA;
		}

		cpu_phandle = of_parse_phandle(cpu_node, "sched-energy-data", 0);
		if (!cpu_phandle) {
			pr_warn("CPU device node has no sched-energy-data\n");
			return -ENODATA;
		}

		table = &per_cpu(energy_table, cpu);
		if (of_property_read_u32(cpu_phandle, "capacity-mips", &table->mips)) {
			pr_warn("No capacity-mips data\n");
			return -ENODATA;
		}

		if (of_property_read_u32(cpu_phandle, "power-coefficient", &table->coefficient)) {
			pr_warn("No power-coefficient data\n");
			return -ENODATA;
		}

		if (of_property_read_u32(cpu_phandle, "static-power-coefficient",
								&table->static_coefficient)) {
			pr_warn("No static-power-coefficient data\n");
			return -ENODATA;
		}

		/*
		 * Data for sse is OPTIONAL.
		 * If it does not fill sse data, sse table and uss table are same.
		 */
		if (of_property_read_u32(cpu_phandle, "capacity-mips-s", &table->mips_s))
			table->mips_s = table->mips;
		if (of_property_read_u32(cpu_phandle, "power-coefficient-s", &table->coefficient_s))
			table->coefficient_s = table->coefficient;

		/*
		 * Capacity-Energy weight is OPTIONAL.
		 * If weight node does not exist, default value of weight is 1.
		 */
		if (of_property_read_u32(cpu_phandle, "capacity-weight",
						&per_cpu(weight, cpu).c_weight))
			per_cpu(weight, cpu).c_weight = 1;
		if (of_property_read_u32(cpu_phandle, "energy-weight",
						&per_cpu(weight, cpu).e_weight))
			per_cpu(weight, cpu).e_weight = 1;
		if (of_property_read_u32(cpu_phandle, "capacity-weight-perf",
						&per_cpu(weight, cpu).c_weight_perf))
			per_cpu(weight, cpu).c_weight_perf = 1;
		if (of_property_read_u32(cpu_phandle, "energy-weight-perf",
						&per_cpu(weight, cpu).e_weight_perf))
			per_cpu(weight, cpu).e_weight_perf = 1;

		of_node_put(cpu_phandle);
		of_node_put(cpu_node);

		pr_info("cpu%d mips=%d coefficient=%d mips_s=%d coefficient_s=%d static_coefficient=%d\n",
			cpu, table->mips, table->coefficient,
			table->mips_s, table->coefficient_s, table->static_coefficient);
	}

	cpufreq_register_notifier(&sched_cpufreq_policy_notifier, CPUFREQ_POLICY_NOTIFIER);

	return 0;
}
core_initcall(init_sched_energy_data);
