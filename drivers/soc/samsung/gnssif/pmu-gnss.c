#include <linux/io.h>
#include <linux/cpumask.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/smc.h>
#include <soc/samsung/exynos-pmu.h>
#include <soc/samsung/cal-if.h>

#include "../cal-if/acpm_dvfs.h"
#include "pmu-gnss.h"
#include "gnss_prj.h"

/* For connectivity I/F */
//#define SMC_CMD_CONN_IF		(0x82000710)
/* Connectivity sub system */
#define EXYNOS_GNSS		(0)
/* Target to set */
#define EXYNOS_SET_CONN_TZPC	(0)

#define gnss_pmu_read	exynos_pmu_read
#define gnss_pmu_write	exynos_pmu_write
#define gnss_pmu_update exynos_pmu_update

#if defined(CONFIG_SOC_EXYNOS9630)
#define BAAW_GNSS_CMGP_ADDR	(0x13FE0000)
#define BAAW_GNSS_CMGP_SIZE	(SZ_64K)

#define BAAW_GNSS_DBUS_ADDR	(0x13FD0000)
#define BAAW_GNSS_DBUS_SIZE	(SZ_64K)

#elif defined(CONFIG_SOC_EXYNOS3830)
#define BAAW_GNSS_CMGP_ADDR	(0x13FE0000)
#define BAAW_GNSS_CMGP_SIZE	(SZ_64K)

#define BAAW_GNSS_DBUS_ADDR	(0x13FD0000)
#define BAAW_GNSS_DBUS_SIZE	(SZ_64K)

#elif defined(CONFIG_SOC_EXYNOS9610)
#define BAAW_GNSS_CMGP_ADDR	(0x13EE0000)
#define BAAW_GNSS_CMGP_SIZE	(SZ_64K)

#define BAAW_GNSS_DBUS_ADDR	(0x13ED0000)
#define BAAW_GNSS_DBUS_SIZE	(SZ_64K)
#endif

static u32 g_shmem_size;
static u32 g_shmem_base;

static void __iomem *baaw_cmgp_reg;
static void __iomem *baaw_dbus_reg;

int gnss_cmgp_read(unsigned int reg_offset, unsigned int *ret)
{
	if (baaw_cmgp_reg == NULL)
		return -EIO;

	*ret = __raw_readl(baaw_cmgp_reg + reg_offset);
	return 0;
}

int gnss_cmgp_write(unsigned int reg_offset, unsigned int val)
{
	unsigned int read_val = 0;

	if (baaw_cmgp_reg == NULL)
		return -EIO;

	__raw_writel(val, baaw_cmgp_reg + reg_offset);
	read_val = __raw_readl(baaw_cmgp_reg + reg_offset);

	if (val != read_val)
		gif_err("ADDR:%08X DATA:%08X => Read to verify:%08X\n", BAAW_GNSS_CMGP_ADDR + reg_offset, val, __raw_readl(baaw_cmgp_reg + reg_offset));

	return 0;
}

int gnss_dbus_read(unsigned int reg_offset, unsigned int *ret)
{
	if (baaw_dbus_reg == NULL)
		return -EIO;

	*ret = __raw_readl(baaw_dbus_reg + reg_offset);
	return 0;
}

int gnss_dbus_write(unsigned int reg_offset, unsigned int val)
{
	unsigned int read_val = 0;

	if (baaw_dbus_reg == NULL)
		return -EIO;

	__raw_writel(val, baaw_dbus_reg + reg_offset);
	read_val = __raw_readl(baaw_dbus_reg + reg_offset);

	if (val != read_val)
		gif_info("ADDR:%08X DATA:%08X => Read to verify:%08X\n", BAAW_GNSS_DBUS_ADDR + reg_offset, val, __raw_readl(baaw_dbus_reg + reg_offset));

	return 0;
}

static int gnss_pmu_clear_interrupt(enum gnss_int_clear gnss_int)
{
	switch (gnss_int) {
	case GNSS_INT_WAKEUP_CLEAR:
		break;
	case GNSS_INT_ACTIVE_CLEAR:
		cal_gnss_active_clear();
		break;
	case GNSS_INT_WDT_RESET_CLEAR:
		break;
	default:
		gif_err("Unexpected interrupt value!\n");
		return -EIO;
	}

	return 0;
}

static int gnss_pmu_release_reset(void)
{
	u32 __maybe_unused gnss_ctrl = 0;
	int ret = 0;

	cal_gnss_reset_release();
	return ret;
}

static int gnss_pmu_hold_reset(void)
{
	int ret = 0;

	cal_gnss_reset_assert();
	mdelay(50);

	return ret;
}

static int gnss_request_tzpc(void)
{
	int ret;

	ret = exynos_smc(SMC_CMD_CONN_IF, (EXYNOS_GNSS << 31) |
			EXYNOS_SET_CONN_TZPC, 0, 0);
	if (ret)
		gif_err("ERR: fail to TZPC setting - %X\n", ret);

	return ret;
}

static void gnss_request_gnss2ap_baaw(void)
{
	gif_info("Config GNSS2AP BAAW\n");

	gif_info("DRAM Configuration\n");
	gnss_dbus_write(0x0, (MEMBASE_GNSS_ADDR >> MEMBASE_ADDR_SHIFT));
	gnss_dbus_write(0x4, (MEMBASE_GNSS_ADDR >> MEMBASE_ADDR_SHIFT)
			+ (g_shmem_size >> MEMBASE_ADDR_SHIFT));
	gnss_dbus_write(0x8, (g_shmem_base >> MEMBASE_ADDR_SHIFT));
	gnss_dbus_write(0xC, 0x80000003);

	gnss_dbus_write(0x10, (MEMBASE_GNSS_ADDR_2ND >> MEMBASE_ADDR_SHIFT));
	gnss_dbus_write(0x14, (MEMBASE_GNSS_ADDR_2ND >> MEMBASE_ADDR_SHIFT)
			+ (g_shmem_size >> MEMBASE_ADDR_SHIFT));
	gnss_dbus_write(0x18, (g_shmem_base >> MEMBASE_ADDR_SHIFT));
	gnss_dbus_write(0x1C, 0x80000003);

	gif_info("MAILBOX CP APM AP CHUB WLBT\n");
	gnss_cmgp_write(0x00, 0x000B1960);	/* GNSS Start address >> 12bit */
	gnss_cmgp_write(0x04, 0x000B19B0);	/* GNSS End address >> 12bit */
	gnss_cmgp_write(0x08, 0x00011960);	/* AP Start address >> 12bit */
	gnss_cmgp_write(0x0C, 0x80000003);

	gif_info("CHUB_SRAM non cachable\n");
	gnss_cmgp_write(0x10, 0x000B0E00);
	gnss_cmgp_write(0x14, 0x000B0E40);
	gnss_cmgp_write(0x18, 0x00010E00);
	gnss_cmgp_write(0x1C, 0x80000003);
}

static int gnss_pmu_power_on(enum gnss_mode mode)
{
	u32 __maybe_unused gnss_ctrl;
	int ret = 0;

	gif_info("mode[%d]\n", mode);

	if (mode == GNSS_POWER_ON) {
		if (cal_gnss_status() > 0) {
			gif_info("GNSS is already Power on, try reset\n");
			cal_gnss_reset_assert();
		} else {
			gif_info("GNSS Power On\n");
			cal_gnss_init();
		}
	} else {
		cal_gnss_reset_release();
	}

	return ret;

}

static int gnss_pmu_init_conf(struct gnss_ctl *gc)
{
	u32 __maybe_unused shmem_size, shmem_base;

	baaw_cmgp_reg = devm_ioremap(gc->dev, BAAW_GNSS_CMGP_ADDR, BAAW_GNSS_CMGP_SIZE);
	if (baaw_cmgp_reg == NULL) {
		gif_err("%s: pmu ioremap failed.\n", gc->pdata->name);
		return -EIO;
	} else
		gif_info("baaw_cmgp_reg : 0x%p\n", baaw_cmgp_reg);

	baaw_dbus_reg = devm_ioremap(gc->dev, BAAW_GNSS_DBUS_ADDR, BAAW_GNSS_DBUS_SIZE);
	if (baaw_dbus_reg == NULL) {
		gif_err("%s: pmu ioremap failed.\n", gc->pdata->name);
		return -EIO;
	} else
		gif_info("baaw_dbus_reg : 0x%p\n", baaw_dbus_reg);

	g_shmem_size = gc->pdata->shmem_size;
	g_shmem_base = gc->pdata->shmem_base;

	gif_info("GNSS SHM address:%X size:%X\n", g_shmem_base, g_shmem_size);

	return 0;
}

static struct gnssctl_pmu_ops pmu_ops = {
	.init_conf = gnss_pmu_init_conf,
	.hold_reset = gnss_pmu_hold_reset,
	.release_reset = gnss_pmu_release_reset,
	.power_on = gnss_pmu_power_on,
	.clear_int = gnss_pmu_clear_interrupt,
	.req_security = gnss_request_tzpc,
	.req_baaw = gnss_request_gnss2ap_baaw,
};

void gnss_get_pmu_ops(struct gnss_ctl *gc)
{
	gc->pmu_ops = &pmu_ops;
	return;
}
