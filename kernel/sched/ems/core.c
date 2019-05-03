/*
 * Core Exynos Mobile Scheduler
 *
 * Copyright (C) 2018 Samsung Electronics Co., Ltd
 * Park Bumgyu <bumgyu.park@samsung.com>
 */


#include "../sched.h"
#include "../tune.h"
#include "ems.h"

#define CREATE_TRACE_POINTS
#include <trace/events/ems.h>

extern void sync_entity_load_avg(struct sched_entity *se);

int exynos_select_task_rq(struct task_struct *p, int prev_cpu,
				int sd_flag, int sync, int wake)
{
	int target_cpu = -1;
	struct enrg_env env = {
		.p = p,
		.prefer_perf = global_boost() || schedtune_prefer_perf(p),
		.prefer_idle = schedtune_prefer_idle(p),
	};

	/*
	 * Update utilization of waking task to apply "sleep" period
	 * before selecting cpu.
	 */
	if (!(sd_flag & SD_BALANCE_FORK))
		sync_entity_load_avg(&p->se);

	if (sysctl_sched_sync_hint_enable && sync) {
		target_cpu = smp_processor_id();
		if (cpumask_test_cpu(target_cpu, &p->cpus_allowed)) {
			trace_ems_select_task_rq(p, target_cpu, wake, "sync");
			return target_cpu;
		}
	}

	cpumask_and(&env.fit_cpus, &p->cpus_allowed, cpu_active_mask);
	ontime_select_fit_cpus(p, &env.fit_cpus);

	/* There is no fit cpus */
	if (cpumask_empty(&env.fit_cpus)) {
		trace_ems_select_task_rq(p, prev_cpu, wake, "no fit cpu");
		return prev_cpu;
	}

	if (!wake) {
		struct cpumask mask;

		/*
		 * 'wake = 0' means that running task is migrated to faster
		 * cpu by ontime migration. If there are fit faster cpus,
		 * current coregroup and env.fit_cpus are exclusive.
		 */
		cpumask_and(&mask, cpu_coregroup_mask(prev_cpu), &env.fit_cpus);
		if (cpumask_weight(&mask)) {
			trace_ems_select_task_rq(p, -1, wake, "no fit faster cpu");
			return -1;
		}
	}

	target_cpu = find_best_cpu(&env);
	if (target_cpu >= 0) {
		trace_ems_select_task_rq(p, target_cpu, wake, "best_cpu");
		return target_cpu;
	}

	target_cpu = prev_cpu;
	trace_ems_select_task_rq(p, target_cpu, wake, "no benefit");

	return target_cpu;
}

static ssize_t show_sched_topology(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int cpu;
	struct sched_domain *sd;
	int ret = 0;

	rcu_read_lock();
	for_each_possible_cpu(cpu) {
		int sched_domain_level = 0;

		sd = rcu_dereference_check_sched_domain(cpu_rq(cpu)->sd);
		while (sd->parent) {
			sched_domain_level++;
			sd = sd->parent;
		}

		for_each_lower_domain(sd) {
			ret += snprintf(buf + ret, 70,
					"[lv%d] cpu%d: flags=%#x sd->span=%#x sg->span=%#x\n",
					sched_domain_level, cpu, sd->flags,
					*(unsigned int *)cpumask_bits(sched_domain_span(sd)),
					*(unsigned int *)cpumask_bits(sched_group_span(sd->groups)));
			sched_domain_level--;
		}
		ret += snprintf(buf + ret,
				50, "----------------------------------------\n");
	}
	rcu_read_unlock();

	return ret;
}

static struct kobj_attribute sched_topology_attr =
__ATTR(sched_topology, 0444, show_sched_topology, NULL);

struct kobject *ems_kobj;

static int __init init_ems_core(void)
{
	int ret;

	ems_kobj = kobject_create_and_add("ems", kernel_kobj);

	ret = sysfs_create_file(ems_kobj, &sched_topology_attr.attr);
	if (ret)
		pr_warn("%s: failed to create sysfs\n", __func__);

	return 0;
}
core_initcall(init_ems_core);

void __init init_ems(void)
{
	init_part();
}
