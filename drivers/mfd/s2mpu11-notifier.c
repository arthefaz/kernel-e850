/*
 * s2mpu11-notifier.c - Interrupt controller support for S2MPU11
 *
 * Copyright (C) 2019 Samsung Electronics Co.Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/err.h>
#include <linux/mfd/samsung/s2mpu11.h>
#include <linux/mfd/samsung/s2mpu11-regulator.h>
#include <linux/notifier.h>
#include <linux/irq.h>

struct notifier_block slave_pmic_notifier;
static struct s2mpu11_dev *s2mpu11_global;

static int s2mps11_notifier_handler(struct notifier_block *nb,
				    unsigned long action,
				    void *data)
{
	pr_info("%s: check handler\n", __func__);

	return IRQ_HANDLED;
}

static BLOCKING_NOTIFIER_HEAD(s2mps11_notifier);

int s2mps11_register_notifier(struct notifier_block *nb,
			      struct s2mpu11_dev *s2mpu11)
{
	int ret;

	ret = blocking_notifier_chain_register(&s2mps11_notifier, nb);
	if (ret < 0)
		pr_err("%s: fail to register notifier\n", __func__);
	return ret;
}

void s2mps11_call_notifier(u8 irq1, u8 irq2, u8 irq3)
{
	pr_info("%s: INT1 = 0x%02x\n", __func__, irq1);
	pr_info("%s: INT2 = 0x%02x\n", __func__, irq2);
	pr_info("%s: INT3 = 0x%02x\n", __func__, irq3);

	blocking_notifier_call_chain(&s2mps11_notifier, 0, s2mpu11_global);
}
EXPORT_SYMBOL(s2mps11_call_notifier);

static const u8 s2mpu11_mask_reg[] = {
	[S2MPU11_PMIC_INT1] = S2MPU11_PMIC_REG_INT1M,
	[S2MPU11_PMIC_INT2] = S2MPU11_PMIC_REG_INT2M,
	[S2MPU11_PMIC_INT3] = S2MPU11_PMIC_REG_INT3M,
};

static void s2mpu11_set_interrupt(struct s2mpu11_dev *s2mpu11)
{
	u8 i, val;

	/* Mask all the interrupt sources */
	for (i = 0; i < S2MPU11_IRQ_GROUP_NR; i++) {
		s2mpu11_write_reg(s2mpu11->pmic, s2mpu11_mask_reg[i], 0xFF);
	}
	s2mpu11_update_reg(s2mpu11->i2c, S2MPU11_PMIC_REG_IRQM,
			   S2MPU11_PM_IRQM_MASK, S2MPU11_PM_IRQM_MASK);

	/* Check unmask interrupt register */
	for (i = 0; i < S2MPU11_IRQ_GROUP_NR; i++) {
		s2mpu11_read_reg(s2mpu11->pmic, s2mpu11_mask_reg[i], &val);
		pr_info("%s: INT%dM = 0x%02x\n", __func__,
			s2mpu11_mask_reg[i] + 1, val);
	}
}

static void s2mpu11_set_notifier(struct s2mpu11_dev *s2mpu11)
{
	slave_pmic_notifier.notifier_call = s2mps11_notifier_handler;
	s2mps11_register_notifier(&slave_pmic_notifier, s2mpu11);
}

int s2mpu11_notifier_init(struct s2mpu11_dev *s2mpu11)
{
	s2mpu11_global = s2mpu11;

	/* register notifier */
	s2mpu11_set_notifier(s2mpu11);

	/* interrupt */
	s2mpu11_set_interrupt(s2mpu11);
	return 0;
}
