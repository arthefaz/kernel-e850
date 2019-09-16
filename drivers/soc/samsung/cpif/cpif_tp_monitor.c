/*
 * Copyright (C) 2019, Samsung Electronics.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hrtimer.h>
#include "modem_prj.h"
#include "modem_utils.h"
#include "link_device_memory.h"
#include "cpif_tp_monitor.h"

static struct cpif_tpmon _tpmon;

/* RX speed */
static unsigned long tpmon_calc_rx_speed_mega_bps(struct cpif_tpmon *tpmon)
{
	unsigned long divider, rx_bytes;

	divider = 131072 * tpmon->interval_sec;	/* 131072 = 1024 * 1024 / 8 */

	if (tpmon->rx_bytes <= tpmon->rx_bytes_prev) {
		tpmon->rx_bytes = 0;
		tpmon->rx_bytes_prev = 0;
	}
	rx_bytes = tpmon->rx_bytes - tpmon->rx_bytes_prev;
	tpmon->rx_bytes_prev = tpmon->rx_bytes;
	if (rx_bytes < divider)
		tpmon->rx_mega_bps = 0;
	else
		tpmon->rx_mega_bps = rx_bytes / divider;

	return tpmon->rx_mega_bps;
}

static bool tpmon_need_to_set_data(struct tpmon_data *data)
{
	int i;

	if (!data->enable)
		return false;

	for (i = 0; i < data->num_threshold; i++)
		if (data->tpmon->rx_mega_bps < data->threshold[i])
			break;

	if (i == data->curr_value)
		return false;

	data->curr_value = i;

	return true;
}

#if defined(CONFIG_RPS)
/* From net/core/net-sysfs.c */
static ssize_t tpmon_store_rps_map(struct netdev_rx_queue *queue, const char *buf, ssize_t len)
{
	struct rps_map *old_map, *map;
	cpumask_var_t mask;
	int err, cpu, i;
	static DEFINE_MUTEX(rps_map_mutex);

	if (!alloc_cpumask_var(&mask, GFP_KERNEL))
		return -ENOMEM;

	err = bitmap_parse(buf, len, cpumask_bits(mask), nr_cpumask_bits);
	if (err) {
		free_cpumask_var(mask);
		return err;
	}

	map = kzalloc(max_t(unsigned int,
			    RPS_MAP_SIZE(cpumask_weight(mask)), L1_CACHE_BYTES),
		      GFP_KERNEL);
	if (!map) {
		free_cpumask_var(mask);
		return -ENOMEM;
	}

	i = 0;
	for_each_cpu_and(cpu, mask, cpu_online_mask)
		map->cpus[i++] = cpu;

	if (i) {
		map->len = i;
	} else {
		kfree(map);
		map = NULL;
	}

	mutex_lock(&rps_map_mutex);
	old_map = rcu_dereference_protected(queue->rps_map,
					    mutex_is_locked(&rps_map_mutex));
	rcu_assign_pointer(queue->rps_map, map);

	if (map)
		static_key_slow_inc(&rps_needed);
	if (old_map)
		static_key_slow_dec(&rps_needed);

	mutex_unlock(&rps_map_mutex);

	if (old_map)
		kfree_rcu(old_map, rcu);

	free_cpumask_var(mask);
	return len;
}

static void tpmon_set_rps(struct tpmon_data *rps_data)
{
	struct io_device *iod;
	int ret = 0;
	char mask[MAX_RPS_STRING];
	unsigned long flags;

	spin_lock_irqsave(&rps_data->tpmon->net_node_lock, flags);

	mif_info("Change RPS at %ldMbps\n", rps_data->tpmon->rx_mega_bps);
	snprintf(mask, MAX_RPS_STRING, "%x", rps_data->values[rps_data->curr_value]);

	list_for_each_entry(iod, &rps_data->tpmon->net_node_list, node_all_ndev) {
		mif_info("RPS[%s] mask:0x%x(%s)\n", iod->name, rps_data->values[rps_data->curr_value], mask);

		if (!iod->name)
			continue;

		ret = (int)tpmon_store_rps_map(&(iod->ndev->_rx[0]), mask, strlen(mask));
		if (ret < 0) {
			mif_err("tpmon_store_rps_map() error:%d\n", ret);
			break;
		}
	}

	spin_unlock_irqrestore(&rps_data->tpmon->net_node_lock, flags);
}
#endif

#if defined(CONFIG_MODEM_IF_NET_GRO)
static void tpmon_set_gro(struct tpmon_data *gro_data)
{
	mif_info("Change GRO at %ldMbps. flush time:%ld\n",
			gro_data->tpmon->rx_mega_bps, gro_data->values[gro_data->curr_value]);
	gro_flush_time = gro_data->values[gro_data->curr_value];
}
#endif

#if defined(CONFIG_MCU_IPC)
static void tpmon_set_irq_affinity(struct tpmon_data *irq_affinity_data)
{
	mif_info("Change IRQ affinity at %ldMbps. CPU:%d\n",
			irq_affinity_data->tpmon->rx_mega_bps,
			irq_affinity_data->values[irq_affinity_data->curr_value]);
	mcu_ipc_set_affinity(0, irq_affinity_data->values[irq_affinity_data->curr_value]);
}
#endif

/* Frequency */
static void tpmon_qos_work(struct work_struct *ws)
{
	struct cpif_tpmon *tpmon = container_of(ws, struct cpif_tpmon, qos_work);

	mif_info("Change freq at %ldMbps. mif_freq:0x%x\n",
			tpmon->rx_mega_bps, tpmon->mif_data.values[tpmon->mif_data.curr_value]);
	pm_qos_update_request(&tpmon->qos_mif, tpmon->mif_data.values[tpmon->mif_data.curr_value]);
}

/* Timer */
static enum hrtimer_restart tpmon_timer(struct hrtimer *timer)
{
	struct cpif_tpmon *tpmon = container_of(timer, struct cpif_tpmon, timer);
	unsigned long flags;

	spin_lock_irqsave(&tpmon->lock, flags);

	tpmon_calc_rx_speed_mega_bps(tpmon);
#if defined(CONFIG_RPS)
	if (tpmon_need_to_set_data(&tpmon->rps_data))
		tpmon_set_rps(&tpmon->rps_data);
#endif
#if defined(CONFIG_MODEM_IF_NET_GRO)
	if (tpmon_need_to_set_data(&tpmon->gro_data))
		tpmon_set_gro(&tpmon->gro_data);
#endif
#if defined(CONFIG_MCU_IPC)
	if (tpmon_need_to_set_data(&tpmon->irq_affinity_data))
		tpmon_set_irq_affinity(&tpmon->irq_affinity_data);
#endif
	if (tpmon_need_to_set_data(&tpmon->mif_data))
		schedule_work(&tpmon->qos_work);

	hrtimer_forward(timer, ktime_get(), ktime_set(tpmon->interval_sec, 0));

	spin_unlock_irqrestore(&tpmon->lock, flags);

	return HRTIMER_RESTART;
}

/*
 * Control
 */
void tpmon_add_rx_bytes(unsigned long bytes)
{
	struct cpif_tpmon *tpmon = &_tpmon;
	unsigned long flags;

	spin_lock_irqsave(&tpmon->lock, flags);

	tpmon->rx_bytes += bytes;

	spin_unlock_irqrestore(&tpmon->lock, flags);
}

void tpmon_add_net_node(struct list_head *node)
{
#if defined(CONFIG_RPS)
	struct cpif_tpmon *tpmon = &_tpmon;
	unsigned long flags;

	spin_lock_irqsave(&tpmon->net_node_lock, flags);

	list_add_tail(node, &tpmon->net_node_list);

	spin_unlock_irqrestore(&tpmon->net_node_lock, flags);
#endif
}

int tpmon_start(u32 interval_sec)
{
	struct cpif_tpmon *tpmon = &_tpmon;
	ktime_t ktime;
	unsigned long flags;

	tpmon_stop();

	spin_lock_irqsave(&tpmon->lock, flags);

	tpmon->rx_bytes = 0;
	tpmon->rx_bytes_prev = 0;
	tpmon->rx_mega_bps = 0;

#if defined(CONFIG_RPS)
	tpmon->rps_data.curr_value = 0;
	tpmon_set_rps(&tpmon->rps_data);
#endif

#if defined(CONFIG_MODEM_IF_NET_GRO)
	tpmon->gro_data.curr_value = 0;
	tpmon_set_gro(&tpmon->gro_data);
#endif

#if defined(CONFIG_MCU_IPC)
	tpmon->irq_affinity_data.curr_value = 0;
	tpmon_set_irq_affinity(&tpmon->irq_affinity_data);
#endif

	tpmon->mif_data.curr_value = 0;
	schedule_work(&tpmon->qos_work);

	tpmon->interval_sec = interval_sec;
	ktime = ktime_set(interval_sec, 0);
	hrtimer_start(&tpmon->timer, ktime, HRTIMER_MODE_REL);

	spin_unlock_irqrestore(&tpmon->lock, flags);

	return 0;
}

int tpmon_stop(void)
{
	struct cpif_tpmon *tpmon = &_tpmon;
	unsigned long flags;

	spin_lock_irqsave(&tpmon->lock, flags);

	hrtimer_cancel(&tpmon->timer);
	flush_workqueue(tpmon->qos_wq);

	spin_unlock_irqrestore(&tpmon->lock, flags);

	return 0;
}

/*
 * Init
 */
static int tpmon_get_dt(struct device_node *np, struct cpif_tpmon *tpmon, struct tpmon_data *data,
			char *threshold_name, char *value_name, char *enable_name)
{
	int ret = 0;
	u32 val;

	data->tpmon = tpmon;

	ret = of_property_read_u32(np, enable_name, &val);
	if (ret) {
		mif_info("%s is not defined\n", enable_name);
		return 0;
	}
	if (!val) {
		mif_info("%s is not enabled\n", enable_name);
		return 0;
	}
	data->enable = true;

	ret = of_property_count_u32_elems(np, threshold_name);
	if (ret < 0) {
		mif_err("of_property_count_u32_elems error:%d(%s)\n", ret, threshold_name);
		return -EINVAL;
	}
	data->num_threshold = ret;
	ret = of_property_read_u32_array(np, threshold_name, data->threshold, data->num_threshold);
	if (ret) {
		mif_err("of_property_read_u32_array error:%d(%s)\n", ret, threshold_name);
		return ret;
	}

	ret = of_property_count_u32_elems(np, value_name);
	if (ret < 0) {
		mif_err("of_property_count_u32_elems error:%d(%s)\n", ret, value_name);
		return -EINVAL;
	}
	ret = of_property_read_u32_array(np, value_name, data->values, ret);
	if (ret) {
		mif_err("of_property_read_u32_array error:%d(%s)\n", ret, value_name);
		return ret;
	}

	return 0;
}

int tpmon_create(struct platform_device *pdev, struct link_device *ld)
{
	struct device_node *np = pdev->dev.of_node;
	struct cpif_tpmon *tpmon = &_tpmon;
	int ret = 0;

	if (!np) {
		mif_err("np is null\n");
		ret = -EINVAL;
		goto create_error;
	}
	if (!ld) {
		mif_err("ld is null\n");
		ret = -EINVAL;
		goto create_error;
	}

	tpmon->ld = ld;

	spin_lock_init(&tpmon->lock);

	hrtimer_init(&tpmon->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tpmon->timer.function = tpmon_timer;

#if defined(CONFIG_RPS)
	spin_lock_init(&tpmon->net_node_lock);
	INIT_LIST_HEAD(&tpmon->net_node_list);

	ret = tpmon_get_dt(np, tpmon, &tpmon->rps_data,
				"tp_rps_threshold", "tp_rps_cpu_mask", "enable_rps_boost");
	if (ret)
		goto create_error;
#endif

#if defined(CONFIG_MODEM_IF_NET_GRO)
	ret = tpmon_get_dt(np, tpmon, &tpmon->gro_data,
				"tp_gro_threshold", "tp_gro_flush_usec", "enable_gro_boost");
	if (ret)
		goto create_error;
#endif

#if defined(CONFIG_MCU_IPC)
	ret = tpmon_get_dt(np, tpmon, &tpmon->irq_affinity_data,
			"tp_irq_affinity_threshold", "tp_irq_affinity_cpu", "enable_irq_affinity_boost");
	if (ret)
		goto create_error;
#endif

	ret = tpmon_get_dt(np, tpmon, &tpmon->mif_data,
			"tp_mif_threshold", "tp_mif_value", "enable_mif_boost");
	if (ret)
		goto create_error;

	pm_qos_add_request(&tpmon->qos_mif, PM_QOS_BUS_THROUGHPUT, 0);
	tpmon->qos_wq = create_singlethread_workqueue("cpif_tpmon_qos_wq");
	if (!tpmon->qos_wq) {
		mif_err("create_singlethread_workqueue() error\n");
		ret = -EINVAL;
		goto create_error;
	}
	INIT_WORK(&tpmon->qos_work, tpmon_qos_work);

	return ret;

create_error:
	mif_err("xxx\n");

	return ret;
}
