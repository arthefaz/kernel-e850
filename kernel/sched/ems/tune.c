/*
 * Frequency variant cpufreq driver
 *
 * Copyright (C) 2017 Samsung Electronics Co., Ltd
 * Park Bumgyu <bumgyu.park@samsung.com>
 */

#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/reciprocal_div.h>

#include <trace/events/ems.h>

#include "../sched.h"
#include "../tune.h"
#include "ems.h"

struct emst_dom {
	int		boost_ratio;	/* boost_ratio of this group at the current freq */
};

struct emst_group {
	struct emst_dom	*dom[NR_CPUS];
	struct kobject	kobj;
};
struct emst_group emst_grp[STUNE_GROUP_COUNT];
bool emst_init_f;

char *stune_group_name[] = {
	"root",
	"foreground",
	"background",
	"top-app",
	"rt",
};

static struct kobject *emst_kobj;
DEFINE_PER_CPU(int, emst_boost_ratio);	/* maximum boot ratio of cpu */

/**********************************************************************
 * common APIs                                                        *
 **********************************************************************/
extern struct reciprocal_value schedtune_spc_rdiv;
unsigned long emst_boost(int cpu, unsigned long util)
{
	int boost = per_cpu(emst_boost_ratio, cpu);
	unsigned long capacity = capacity_cpu(cpu, 0);
	unsigned long boosted_util = 0;
	long long margin = 0;

	if (!boost || util >= capacity)
		return util;

	/*
	 * Signal proportional compensation (SPC)
	 *
	 * The Boost (B) value is used to compute a Margin (M) which is
	 * proportional to the complement of the original Signal (S):
	 *   M = B * (capacity - S)
	 * The obtained M could be used by the caller to "boost" S.
	 */
	if (boost >= 0) {
		margin  = capacity - util;
		margin *= boost;
	} else
		margin = -util * boost;

	margin  = reciprocal_divide(margin, schedtune_spc_rdiv);

	if (boost < 0)
		margin *= -1;

	boosted_util = util + margin;

	trace_emst_boost(cpu, boost, util, boosted_util);

	return boosted_util;
}

/* Update maximum values of boost groups of this cpu */
void emst_cpu_update(int cpu, u64 now)
{
	int idx, boost_ratio_max;
	if (unlikely(!emst_init_f))
		return;

	/* The root boost group is always active */
	boost_ratio_max = emst_grp[0].dom[cpu]->boost_ratio;
	for (idx = 1; idx < STUNE_GROUP_COUNT; ++idx) {
		int val;
		if (schedtune_cpu_boost_group_active(idx, cpu, now))
			continue;

		/* This boost group is active and has more high value */
		val = emst_grp[idx].dom[cpu]->boost_ratio;
		if (boost_ratio_max < val)
			boost_ratio_max = val;
	}

	/*
	 * Ensures grp_boost_max is non-negative when all cgroup boost values
	 * are neagtive. Avoids under-accounting of cpu capacity which may cause
	 * task stacking and frequency spikes.
	 */
	per_cpu(emst_boost_ratio, cpu) = max(boost_ratio_max, 0);
}

/**********************************************************************
 * Sysfs	                                                      *
 **********************************************************************/
struct emst_attr {
	struct attribute attr;
	ssize_t (*show)(struct kobject *, char *);
	ssize_t (*store)(struct kobject *, const char *, size_t count);
};

#define emst_attr_rw(name)				\
static struct emst_attr name##_attr =			\
__ATTR(name, 0644, show_##name, store_##name)

#define emst_show(name, type)							\
static ssize_t show_##name(struct kobject *k, char *buf)			\
{										\
	struct emst_group *group = container_of(k, struct emst_group, kobj);	\
	int cpu, ret = 0;							\
										\
	for_each_possible_cpu(cpu) {						\
		if (cpu != cpumask_first(cpu_coregroup_mask(cpu)))		\
			continue;						\
		ret += sprintf(buf + ret, "cpu:%d ratio%3d\n",			\
					cpu, group->dom[cpu]->type);		\
	}									\
										\
	return ret;								\
}

#define emst_store(name, type)						\
static ssize_t store_##name(struct kobject *k, const char *buf, size_t count)	\
{										\
	struct emst_group *group = container_of(k, struct emst_group, kobj);	\
	int cpu, val;							\
										\
	if (!sscanf(buf, "%d %d", &cpu, &val))					\
		return -EINVAL;							\
										\
	if (cpu < 0 || cpu >= NR_CPUS || val < -200 || val > 500)		\
		return -EINVAL;							\
										\
	group->dom[cpu]->type = val;						\
	return count;								\
}

emst_store(boost, boost_ratio);
emst_show(boost, boost_ratio);
emst_attr_rw(boost);

static ssize_t show(struct kobject *kobj, struct attribute *at, char *buf)
{
	struct emst_attr *attr = container_of(at, struct emst_attr, attr);
	return attr->show(kobj, buf);
}

static ssize_t store(struct kobject *kobj, struct attribute *at,
					const char *buf, size_t count)
{
	struct emst_attr *attr = container_of(at, struct emst_attr, attr);
	return attr->store(kobj, buf, count);
}

static const struct sysfs_ops emst_sysfs_ops = {
	.show	= show,
	.store	= store,
};

static struct attribute *emst_attrs[] = {
	&boost_attr.attr,
	NULL
};

static struct kobj_type ktype_fb = {
	.sysfs_ops	= &emst_sysfs_ops,
	.default_attrs	= emst_attrs,
};

/**********************************************************************
 * initialization                                                     *
 **********************************************************************/
static int __init emst_init(void)
{
	struct device_node *ems_dn, *emst_dn, *dn;
	struct emst_dom *cur;
	int idx;
	char name[15];
	u32 val[NR_CPUS];

	emst_kobj = kobject_create_and_add("emst", ems_kobj);
	if (!emst_kobj)
		return -EINVAL;

	ems_dn = of_find_node_by_path("/cpus/ems");
	if (!ems_dn)
		return -EINVAL;

	emst_dn = of_find_node_by_name(ems_dn, "ems-tune");
	if (!emst_dn)
		return -EINVAL;

	for (idx = 0; idx < STUNE_GROUP_COUNT; ++idx) {
		int cnt, cpu, tmp;

		/* get the stune group name */
		snprintf(name, sizeof(name), "%s", stune_group_name[idx]);

		/* find node for this group */
		dn = of_find_node_by_name(emst_dn, name);
		if (!dn) {
			pr_warn("%s: %s is not registered on the DT\n", __func__, name);
			continue;
		}

		/* connect kobject */
		if (kobject_init_and_add(&emst_grp[idx].kobj, &ktype_fb, emst_kobj, name))
			pr_warn("%s: failed to initialize kobject of %s\n", __func__, name);

		/* allocate emst_dom */
		cnt = 0;
		for_each_possible_cpu(cpu) {
			if (cpu != cpumask_first(cpu_coregroup_mask(cpu)))
				continue;

			cur = kzalloc(sizeof(struct emst_dom), GFP_KERNEL);
			if (!cur) {
				pr_warn("%s: failed to allocate emst_dom\n", __func__);
				continue;
			}

			for_each_cpu(tmp, cpu_coregroup_mask(cpu))
				emst_grp[idx].dom[tmp] = cur;

			cnt++;
		}

		/* parse boost ratio */
		if (of_property_read_u32_array(dn, "boost_ratio", (u32 *)&val, cnt))
			continue;

		cnt = 0;
		for_each_possible_cpu(cpu) {
			if (cpu != cpumask_first(cpu_coregroup_mask(cpu)))
				continue;

			emst_grp[idx].dom[cpu]->boost_ratio = val[cnt];
			cnt++;
		}
	}

	emst_init_f = true;

	return 0;
}
late_initcall(emst_init);
