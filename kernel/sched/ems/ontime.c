/*
 * On-time Migration Feature for Exynos Mobile Scheduler (EMS)
 *
 * Copyright (C) 2018 Samsung Electronics Co., Ltd
 * LEE DAEYEONG <daeyeong.lee@samsung.com>
 */

#include <trace/events/ems.h>

#include "../sched.h"
#include "../tune.h"
<<<<<<< HEAD
#include "./ems.h"
=======
#include "ems.h"
>>>>>>> 78ae72181ddd... sched: support EMSv2.0

/****************************************************************/
/*			On-time migration			*/
/****************************************************************/
#define TASK_TRACK_COUNT	5
#define MIN_CAPACITY_CPU	0

#define ontime_of(p)		(&p->se.ontime)

#define cap_scale(v, s)		((v)*(s) >> SCHED_CAPACITY_SHIFT)

#define entity_is_cfs_rq(se)	(se->my_q)
#define entity_is_task(se)	(!se->my_q)

/* Structure of ontime migration condition */
struct ontime_cond {
	bool			enabled;

	unsigned long		upper_boundary;
	unsigned long		lower_boundary;
	unsigned long		upper_boundary_s;
	unsigned long		lower_boundary_s;

	int			coregroup;
	struct cpumask		cpus;

	struct list_head	list;

	/* kobject for sysfs group */
	struct kobject		kobj;
};
LIST_HEAD(cond_list);

/* Structure of ontime migration environment */
struct ontime_env {
	struct rq		*dst_rq;
	int			dst_cpu;
	struct rq		*src_rq;
	int			src_cpu;
	struct task_struct	*target_task;
	int			boost_migration;
};
DEFINE_PER_CPU(struct ontime_env, ontime_env);

static inline struct sched_entity *se_of(struct sched_avg *sa)
{
	return container_of(sa, struct sched_entity, avg);
}

struct ontime_cond *get_current_cond(int cpu)
{
	struct ontime_cond *curr;

	list_for_each_entry(curr, &cond_list, list) {
		if (cpumask_test_cpu(cpu, &curr->cpus))
			return curr;
	}

	return NULL;
}

#define u_boundary(cond, p)	(p->sse ? cond->upper_boundary_s : cond->upper_boundary)
#define l_boundary(cond, p)	(p->sse ? cond->lower_boundary_s : cond->lower_boundary)

unsigned long get_upper_boundary(int cpu, struct task_struct *p)
{
	struct ontime_cond *curr = get_current_cond(cpu);

	if (curr)
		return u_boundary(curr, p);
	else
		return ULONG_MAX;
}

static inline unsigned long get_lower_boundary(int cpu, struct task_struct *p)
{
	struct ontime_cond *curr = get_current_cond(cpu);

	if (curr)
		return l_boundary(curr, p);
	else
		return 0;
}

static inline int check_migrate_faster(int src, int dst, int sse)
{
	if (capacity_cpu(src, sse) < capacity_cpu(dst, sse))
		return true;
	else
		return false;
}

static inline int check_migrate_slower(int src, int dst, int sse)
{
	if (capacity_cpu(src, sse) > capacity_cpu(dst, sse))
		return true;
	else
		return false;
}

void ontime_select_fit_cpus(struct task_struct *p, struct cpumask *fit_cpus)
{
	struct ontime_cond *curr;
	int src_cpu = task_cpu(p);
	u32 runnable = ml_task_runnable(p);
	struct cpumask mask;

	curr = get_current_cond(src_cpu);
	if (!curr)
		return;

	cpumask_clear(&mask);

	if (runnable >= u_boundary(curr, p)) {
		/*
		 * Runnable of the task is bigger than upper boundary of
		 * current, faster cpus are marked on the mask.
		 */
		list_for_each_entry_continue(curr, &cond_list, list) {
			int dst_cpu = cpumask_any(&curr->cpus);

			if (check_migrate_faster(src_cpu, dst_cpu, p->sse))
				cpumask_or(&mask, &mask, &curr->cpus);
		}
	} else if (runnable >= l_boundary(curr, p)) {
		/*
		 * Runnable of the task is in the current boundary,
		 * current coregroup are marked on the mask.
		 */
		cpumask_copy(&mask, &curr->cpus);
	} else {
		/*
		 * Runnable of the task is smaller than lower boundary of
		 * current, give up cpu selection.
		 */
		return;
	}

	cpumask_and(fit_cpus, fit_cpus, &mask);

	trace_ems_ontime_fit_cpus(p, *(unsigned int *)cpumask_bits(fit_cpus));
}

extern struct sched_entity *__pick_next_entity(struct sched_entity *se);
static struct task_struct *
pick_heavy_task(struct sched_entity *se, int *boost_migration)
{
	struct task_struct *heaviest_task = NULL;
	struct task_struct *p = container_of(se, struct task_struct, se);
	u32 runnable, max_ratio = 0;
	int task_count = 0;

	/*
	 * Since current task does not exist in entity list of cfs_rq,
	 * check first that current task is heavy.
	 */
	if (global_boost() || schedtune_prefer_perf(p)) {
		*boost_migration = 1;
		return p;
	}

	if (schedtune_ontime(p)) {
		runnable = ml_task_runnable(p);
		if (runnable >= get_upper_boundary(task_cpu(p), p)) {
			heaviest_task = p;
			max_ratio = runnable * 100 / capacity_cpu_orig(task_cpu(p), p->sse);
			*boost_migration = 0;
		}
	}

	se = __pick_first_entity(se->cfs_rq);
	while (se && task_count < TASK_TRACK_COUNT) {
		int task_ratio;

		/* Skip non-task entity */
		if (entity_is_cfs_rq(se))
			goto next_entity;

		p = container_of(se, struct task_struct, se);
		if (schedtune_prefer_perf(p)) {
			heaviest_task = p;
			*boost_migration = 1;
			break;
		}

		if (!schedtune_ontime(p))
			goto next_entity;

		runnable = ml_task_runnable(p);
		if (runnable < get_upper_boundary(task_cpu(p), p))
			goto next_entity;

		task_ratio = runnable * 100 / capacity_cpu_orig(task_cpu(p), p->sse);
		if (task_ratio > max_ratio) {
			heaviest_task = p;
			max_ratio = task_ratio;
			*boost_migration = 0;
		}

next_entity:
		se = __pick_next_entity(se);
		task_count++;
	}

	return heaviest_task;
}

static bool can_migrate(struct task_struct *p, struct ontime_env *env)
{
	struct rq *src_rq = env->src_rq;
	int src_cpu = env->src_cpu;

	if (ontime_of(p)->migrating == 0)
		return false;

	if (p->exit_state)
		return false;

	if (unlikely(src_rq != task_rq(p)))
		return false;

	if (unlikely(src_cpu != smp_processor_id()))
		return false;

	if (src_rq->nr_running <= 1)
		return false;

	if (!cpumask_test_cpu(env->dst_cpu, &p->cpus_allowed))
		return false;

	if (task_running(env->src_rq, p))
		return false;

	return true;
}

static void move_task(struct task_struct *p, struct ontime_env *env)
{
	p->on_rq = TASK_ON_RQ_MIGRATING;
	deactivate_task(env->src_rq, p, 0);
	set_task_cpu(p, env->dst_cpu);

	activate_task(env->dst_rq, p, 0);
	p->on_rq = TASK_ON_RQ_QUEUED;
	check_preempt_curr(env->dst_rq, p, 0);
}

static int move_specific_task(struct task_struct *target, struct ontime_env *env)
{
	struct list_head *tasks = lb_cfs_tasks(env->src_rq, target->sse);
	struct task_struct *p, *n;

	list_for_each_entry_safe(p, n, tasks, se.group_node) {
		if (p != target)
			continue;

		move_task(p, env);
		return 1;
	}

	return 0;
}

static int ontime_migration_cpu_stop(void *data)
{
	struct ontime_env *env = data;
	struct rq *src_rq, *dst_rq;
	struct task_struct *p;
	int src_cpu, dst_cpu;
	int boost_migration;

	/* Initialize environment data */
	src_rq = env->src_rq;
	dst_rq = env->dst_rq = cpu_rq(env->dst_cpu);
	src_cpu = env->src_cpu = env->src_rq->cpu;
	dst_cpu = env->dst_cpu;
	p = env->target_task;
	boost_migration = env->boost_migration;

	raw_spin_lock_irq(&src_rq->lock);

	/* Check task can be migrated */
	if (!can_migrate(p, env))
		goto out_unlock;

	BUG_ON(src_rq == dst_rq);

	/* Move task from source to destination */
	double_lock_balance(src_rq, dst_rq);
	if (move_specific_task(p, env)) {
		trace_ems_ontime_migration(p, ml_task_runnable(p),
				src_cpu, dst_cpu, boost_migration);
	}
	double_unlock_balance(src_rq, dst_rq);

out_unlock:
	ontime_of(p)->migrating = 0;

	src_rq->active_balance = 0;
	dst_rq->ontime_migrating = 0;

	raw_spin_unlock_irq(&src_rq->lock);
	put_task_struct(p);

	return 0;
}

/****************************************************************/
/*			External APIs				*/
/****************************************************************/
DEFINE_PER_CPU(struct cpu_stop_work, ontime_migration_work);
static DEFINE_SPINLOCK(om_lock);

void ontime_migration(void)
{
	int cpu;

	if (!spin_trylock(&om_lock))
		return;

	for_each_cpu(cpu, cpu_active_mask) {
		unsigned long flags;
		struct rq *rq = cpu_rq(cpu);
		struct sched_entity *se;
		struct task_struct *p;
		struct ontime_env *env = &per_cpu(ontime_env, cpu);
		int boost_migration = 0;
		int dst_cpu;

		raw_spin_lock_irqsave(&rq->lock, flags);

		/*
		 * Ontime migration is not performed when active balance
		 * is in progress.
		 */
		if (rq->active_balance) {
			raw_spin_unlock_irqrestore(&rq->lock, flags);
			continue;
		}

		/*
		 * No need to migration if source cpu does not have cfs
		 * tasks.
		 */
		if (!rq->cfs.curr) {
			raw_spin_unlock_irqrestore(&rq->lock, flags);
			continue;
		}

		/* Find task entity if entity is cfs_rq. */
		se = rq->cfs.curr;
		if (entity_is_cfs_rq(se)) {
			struct cfs_rq *cfs_rq = se->my_q;

			while (cfs_rq) {
				se = cfs_rq->curr;
				cfs_rq = se->my_q;
			}
		}

		/*
		 * Pick task to be migrated. Return NULL if there is no
		 * heavy task in rq.
		 */
		p = pick_heavy_task(se, &boost_migration);
		if (!p) {
			raw_spin_unlock_irqrestore(&rq->lock, flags);
			continue;
		}

		/* Select destination cpu which the task will be moved */
		dst_cpu = exynos_select_task_rq(p, cpu, 0, 0, 0);
		if (dst_cpu < 0 || cpu == dst_cpu) {
			raw_spin_unlock_irqrestore(&rq->lock, flags);
			continue;
		}

		ontime_of(p)->migrating = 1;
		get_task_struct(p);

		/* Set environment data */
		env->dst_cpu = dst_cpu;
		env->src_rq = rq;
		env->target_task = p;
		env->boost_migration = boost_migration;

		/* Prevent active balance to use stopper for migration */
		rq->active_balance = 1;

		cpu_rq(dst_cpu)->ontime_migrating = 1;

		raw_spin_unlock_irqrestore(&rq->lock, flags);

		/* Migrate task through stopper */
		stop_one_cpu_nowait(cpu, ontime_migration_cpu_stop, env,
				&per_cpu(ontime_migration_work, cpu));
	}

	spin_unlock(&om_lock);
}

int ontime_can_migrate_task(struct task_struct *p, int dst_cpu)
{
	int src_cpu = task_cpu(p);
	u32 runnable;

	if (!schedtune_ontime(p))
		return true;

	if (ontime_of(p)->migrating == 1) {
		trace_ems_ontime_can_migrate(p, src_cpu, dst_cpu, false, "on migrating");
		return false;
	}

	/*
	 * Task is heavy enough but load balancer tries to migrate the task to
	 * slower cpu, it does not allow migration.
	 */
	runnable = ml_task_runnable(p);
	if (runnable >= get_lower_boundary(src_cpu, p) &&
	    check_migrate_slower(src_cpu, dst_cpu, p->sse) > 0) {
		/*
		 * However, only if the source cpu is overutilized, it allows
		 * migration if the task is not very heavy.
		 * (criteria : task util is under 50% of cpu util)
		 */
		if ((capacity_cpu(src_cpu, 0) * 1024) < (ml_cpu_util(src_cpu) * 1280) &&
		    ml_task_util(p) < (_ml_cpu_util(src_cpu, p->sse) >> 1)) {
			trace_ems_ontime_can_migrate(p, src_cpu, dst_cpu, true, "overutil");
			return true;
		}

		trace_ems_ontime_can_migrate(p, src_cpu, dst_cpu, false, "migrate to slower");
		return false;
	}

	trace_ems_ontime_can_migrate(p, src_cpu, dst_cpu, true, "n/a");

	return true;
}

/****************************************************************/
/*				SYSFS				*/
/****************************************************************/
static struct kobject *ontime_kobj;

struct ontime_attr {
	struct attribute attr;
	ssize_t (*show)(struct kobject *, char *);
	ssize_t (*store)(struct kobject *, const char *, size_t count);
};

#define show_store_attr(_name, _type, _max)					\
static ssize_t show_##_name(struct kobject *k, char *buf)			\
{										\
	struct ontime_cond *cond = container_of(k, struct ontime_cond, kobj);	\
										\
	return sprintf(buf, "%u\n", (unsigned int)cond->_name);			\
}										\
										\
static ssize_t store_##_name(struct kobject *k, const char *buf, size_t count)	\
{										\
	unsigned int val;							\
	struct ontime_cond *cond = container_of(k, struct ontime_cond, kobj);	\
										\
	if (!sscanf(buf, "%u", &val))						\
		return -EINVAL;							\
										\
	val = val > _max ? _max : val;						\
	cond->_name = (_type)val;						\
										\
	return count;								\
}										\
										\
static struct ontime_attr _name##_attr =					\
__ATTR(_name, 0644, show_##_name, store_##_name)

show_store_attr(upper_boundary, unsigned long, 1024);
show_store_attr(lower_boundary, unsigned long, 1024);
show_store_attr(upper_boundary_s, unsigned long, 1024);
show_store_attr(lower_boundary_s, unsigned long, 1024);

static ssize_t show(struct kobject *kobj, struct attribute *at, char *buf)
{
	struct ontime_attr *oattr = container_of(at, struct ontime_attr, attr);

	return oattr->show(kobj, buf);
}

static ssize_t store(struct kobject *kobj, struct attribute *at,
		     const char *buf, size_t count)
{
	struct ontime_attr *oattr = container_of(at, struct ontime_attr, attr);

	return oattr->store(kobj, buf, count);
}

static const struct sysfs_ops ontime_sysfs_ops = {
	.show	= show,
	.store	= store,
};

static struct attribute *ontime_attrs[] = {
	&upper_boundary_attr.attr,
	&lower_boundary_attr.attr,
	&upper_boundary_s_attr.attr,
	&lower_boundary_s_attr.attr,
	NULL
};

static struct kobj_type ktype_ontime = {
	.sysfs_ops	= &ontime_sysfs_ops,
	.default_attrs	= ontime_attrs,
};

static int __init ontime_sysfs_init(void)
{
	struct ontime_cond *curr;

	if (list_empty(&cond_list))
		return 0;

	ontime_kobj = kobject_create_and_add("ontime", ems_kobj);
	if (!ontime_kobj)
		goto out;

	/* Add ontime sysfs node for each coregroup */
	list_for_each_entry(curr, &cond_list, list) {
		int ret;

		/* If ontime is disabled in this coregroup, do not create sysfs node */
		if (!curr->enabled)
			continue;

		ret = kobject_init_and_add(&curr->kobj, &ktype_ontime,
				ontime_kobj, "coregroup%d", curr->coregroup);
		if (ret)
			goto out;
	}

	return 0;

out:
	pr_err("ONTIME(%s): failed to create sysfs node\n", __func__);
	return -EINVAL;
}

/****************************************************************/
/*			initialization				*/
/****************************************************************/
static void __init
parse_ontime(struct device_node *dn, struct ontime_cond *cond, int cnt)
{
	struct device_node *ontime, *coregroup;
	char name[15];
	unsigned long capacity, capacity_s;
	int prop;
	int res = 0;

	ontime = of_get_child_by_name(dn, "ontime");
	if (!ontime)
		goto disable;

	snprintf(name, sizeof(name), "coregroup%d", cnt);
	coregroup = of_get_child_by_name(ontime, name);
	if (!coregroup)
		goto disable;
	cond->coregroup = cnt;

	capacity = capacity_cpu_orig(cpumask_first(&cond->cpus), 0);
	capacity_s = capacity_cpu_orig(cpumask_first(&cond->cpus), 1);

	/* If capacity of this coregroup is 0, disable ontime of this coregroup */
	if (capacity == 0)
		goto disable;

	/* If any of ontime parameter isn't, disable ontime of this coregroup */
	res |= of_property_read_s32(coregroup, "upper-boundary", &prop);
	cond->upper_boundary = prop;

	res |= of_property_read_s32(coregroup, "lower-boundary", &prop);
	cond->lower_boundary = prop;

	res |= of_property_read_s32(coregroup, "upper-boundary-s", &prop);
	cond->upper_boundary_s = prop;

	res |= of_property_read_s32(coregroup, "lower-boundary-s", &prop);
	cond->lower_boundary_s = prop;

	if (res)
		goto disable;

	cond->enabled = true;
	return;

disable:
	pr_err("ONTIME(%s): failed to parse ontime node\n", __func__);
	cond->enabled = false;
	cond->upper_boundary = ULONG_MAX;
	cond->lower_boundary = 0;
	cond->upper_boundary_s = ULONG_MAX;
	cond->lower_boundary_s = 0;
}

static int __init init_ontime(void)
{
	struct ontime_cond *cond;
	struct device_node *dn;
	int cpu, cnt = 0;

	INIT_LIST_HEAD(&cond_list);

	dn = of_find_node_by_path("/cpus/ems");
	if (!dn)
		return 0;

	if (!cpumask_equal(cpu_possible_mask, cpu_all_mask))
		return 0;

	for_each_possible_cpu(cpu) {
		if (cpu != cpumask_first(cpu_coregroup_mask(cpu)))
			continue;

		cond = kzalloc(sizeof(struct ontime_cond), GFP_KERNEL);

		cpumask_copy(&cond->cpus, cpu_coregroup_mask(cpu));

		parse_ontime(dn, cond, cnt++);

		list_add_tail(&cond->list, &cond_list);
	}

	ontime_sysfs_init();

	of_node_put(dn);
	return 0;
}
late_initcall(init_ontime);
