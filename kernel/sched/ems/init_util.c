/*
 * task util initialization
 *
 * Copyright (C) 2018 Samsung Electronics Co., Ltd
 * Lakkyung Jung <lakkyung.jung@samsung.com>
 */

#include <linux/sched.h>
#include <trace/events/ems.h>

#include "../sched.h"
<<<<<<< HEAD
=======
#include "../sched-pelt.h"
#include "ems.h"
>>>>>>> 78ae72181ddd... sched: support EMSv2.0

enum {
	INHERIT_CFS_RQ = 0,
	INHERIT_PARENT,
	INHERIT_MAX_NUM,
};

static u32 inherit_type = INHERIT_CFS_RQ;
static u32 inherit_ratio = 25;	/* EMS default : 25% (fair : 50%) */

static void inherit_cfs_rq_util(struct sched_entity *se)
{
	struct cfs_rq *cfs_rq = se->cfs_rq;
	struct sched_avg *sa = &se->avg;
	unsigned long cpu_scale = arch_scale_cpu_capacity(NULL, cpu_of(cfs_rq->rq));
	long cap = (long)(cpu_scale - cfs_rq->avg.util_avg);

	if (cap > 0) {
		if (cfs_rq->avg.util_avg != 0) {
			sa->util_avg  = cfs_rq->avg.util_avg * se->load.weight;
			sa->util_avg /= (cfs_rq->avg.load_avg + 1);
			sa->util_avg = sa->util_avg << 1;

			if (sa->util_avg > cap)
				sa->util_avg = cap;
		} else {
			sa->util_avg = cap * inherit_ratio / 100;
		}
		sa->util_sum = sa->util_avg * LOAD_AVG_MAX;
	}
}

static void inherit_parent_util(struct sched_entity *se)
{
	se->avg.util_avg = current->se.avg.util_avg * inherit_ratio / 100;
	se->avg.util_sum = current->se.avg.util_sum * inherit_ratio / 100;
}

void exynos_post_init_entity_util_avg(struct sched_entity *se)
{
	switch(inherit_type) {
	case INHERIT_CFS_RQ:
		inherit_cfs_rq_util(se);
		post_init_multi_load_cfs_rq(se, inherit_ratio);
		break;
	case INHERIT_PARENT:
		inherit_parent_util(se);
		post_init_multi_load_parent(se, inherit_ratio);
		break;
	default:
		pr_info("%s: Not support initial util type %d\n",
				__func__, inherit_type);
	}
}

static ssize_t show_inherit_type(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
        return snprintf(buf, 10, "%d\n", inherit_type);
}

static ssize_t store_inherit_type(struct kobject *kobj,
                struct kobj_attribute *attr, const char *buf,
                size_t count)
{
        int input;

        if (!sscanf(buf, "%d", &input))
                return -EINVAL;

        input = min_t(u32, input, INHERIT_CFS_RQ);
	input = max_t(u32, input, INHERIT_MAX_NUM - 1);

        inherit_type = input;

        return count;
}

static ssize_t show_inherit_ratio(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
        return snprintf(buf, 10, "%d\n", inherit_ratio);
}

static ssize_t store_inherit_ratio(struct kobject *kobj,
                struct kobj_attribute *attr, const char *buf,
                size_t count)
{
        int input;

        if (!sscanf(buf, "%d", &input))
                return -EINVAL;

        inherit_ratio = !!input;

        return count;
}

static struct kobj_attribute inherit_type_attr =
__ATTR(inherit_type, 0644, show_inherit_type, store_inherit_type);

static struct kobj_attribute inherit_ratio_attr =
__ATTR(inherit_ratio, 0644, show_inherit_ratio, store_inherit_ratio);

static struct attribute *attrs[] = {
	&inherit_type_attr.attr,
	&inherit_ratio_attr.attr,
	NULL,
};

static const struct attribute_group attr_group = {
	.attrs = attrs,
};

static int __init init_util_sysfs(void)
{
	struct kobject *kobj;

	kobj = kobject_create_and_add("init_util", ems_kobj);
	if (!kobj)
		return -EINVAL;

	if (sysfs_create_group(kobj, &attr_group))
		return -EINVAL;

	return 0;
}
late_initcall(init_util_sysfs);
