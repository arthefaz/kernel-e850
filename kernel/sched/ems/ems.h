/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

extern struct kobject *ems_kobj;

struct enrg_env {
	struct task_struct *p;

	int prefer_perf;
	int prefer_idle;

	struct cpumask fit_cpus;
	struct cpumask candidates;
	struct cpumask idle_candidates;

	unsigned long c_weight[NR_CPUS];	/* capacity weight */
	unsigned long e_weight[NR_CPUS];	/* energy weight */

	int wake;
};


/* ISA flags */
#define USS	0
#define SSE	1


/* EMS service */
extern int select_service_cpu(struct task_struct *p);

/* energy model */
extern unsigned long capacity_cpu_orig(int cpu, int sse);
extern unsigned long capacity_cpu(int cpu, int sse);
extern unsigned long capacity_ratio(int cpu, int sse);

/* multi load */
extern unsigned long ml_task_util(struct task_struct *p);
extern unsigned long ml_task_runnable(struct task_struct *p);
extern unsigned long ml_task_util_est(struct task_struct *p);
extern unsigned long ml_boosted_task_util(struct task_struct *p);
extern unsigned long ml_cpu_util(int cpu);
extern unsigned long _ml_cpu_util(int cpu, int sse);
extern unsigned long ml_cpu_util_ratio(int cpu, int sse);
extern unsigned long __ml_cpu_util_with(int cpu, struct task_struct *p, int sse);
extern unsigned long ml_cpu_util_with(int cpu, struct task_struct *p);
extern unsigned long ml_cpu_util_without(int cpu, struct task_struct *p);
extern void post_init_multi_load_cfs_rq(struct sched_entity *se, u32 inherit_ratio);
extern void post_init_multi_load_parent(struct sched_entity *se, u32 inherit_ratio);
extern void init_part(void);

/* efficiency cpu selection */
extern int find_best_cpu(struct enrg_env *env);

/* ontime migration */
extern void ontime_select_fit_cpus(struct task_struct *p, struct cpumask *fit_cpus);
extern unsigned long get_upper_boundary(int cpu, struct task_struct *p);

/* global boost */
extern int global_boost(void);


static inline cpu_overutilized(unsigned long capacity, unsigned long util)
{
	return (capacity * 1024) < (util * 1280);
}
