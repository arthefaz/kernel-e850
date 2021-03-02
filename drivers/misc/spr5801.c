#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/kthread.h>

/* Must be > 10 us, w.r.t. datasheet */
#define SPR5801_RESET_TIME		200	/* usec */
/* E-fuse loading time (after reset), ~200 usec by datasheet */
#define SPR5801_EFUSE_TIME		200	/* usec */

struct spr5801 {
	struct i2c_client *client;
	struct gpio_desc *reset_gpio;
	struct task_struct *thread;
};

#if 0
static int spr5801_thread_func(void *data)
{
	struct spr5801 *obj = data;

	while (!kthread_should_stop()) {
		/*
		 * Pin configured as active-low, open-drain and pulled-up:
		 *
		 *   0 = 1.8V (via internal pull-up)
		 *   1 = 0V (via Open-Drain buffer)
		 */
		gpiod_set_value(obj->reset_gpio, 0);
		msleep(1000);
		gpiod_set_value(obj->reset_gpio, 1);
		msleep(1000);
	}

	return 0;
}
#endif

/* Reset AI chip */
static void spr5801_reset(struct spr5801 *obj)
{

	/*
	 * Pin configured as active-low, open-drain and pulled-up:
	 *
	 *   0 = 1.8V (via internal pull-up)
	 *   1 = 0V (via Open-Drain buffer)
	 */
	gpiod_set_value(obj->reset_gpio, 1);
	udelay(SPR5801_RESET_TIME);
	gpiod_set_value(obj->reset_gpio, 0);
	udelay(SPR5801_EFUSE_TIME);
}

#if 0
/* Read something from SPR5801 chip via I2C */
static void spr5801_read_chip_id(struct spr5801 *obj)
{
	struct device *dev = &obj->client->dev;
	s32 chip_id_high, chip_id_low;
	u16 chip_id;

	/* Read Chip ID register */
	chip_id_high = i2c_smbus_read_byte_data(obj->client, 0x00);
	if (chip_id_high < 0) {
		dev_err(dev, "### Unable to read I2C data from SPR5801 chip!\n");
		return;
	}
	chip_id_low = i2c_smbus_read_byte_data(obj->client, 0x01);

	chip_id = (u8)chip_id_low | (u8)chip_id_high << 8;
	pr_err("### Read data: chip id = 0x%x (%d)\n", chip_id, chip_id);
}
#endif

static int spr5801_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct spr5801 *obj;
	struct device *dev = &client->dev;
	int ret;

	pr_err("### %s: probe()\n", __func__);

	obj = devm_kzalloc(dev, sizeof(*obj), GFP_KERNEL);
	if (!obj)
		return -ENOMEM;

	dev_set_drvdata(dev, obj);

	obj->client = client;

	obj->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_IN);
	if (IS_ERR(obj->reset_gpio)) {
		ret = PTR_ERR(obj->reset_gpio);
		dev_err(dev, "### Failed to get reset gpio; ret = %d\n", ret);
		return ret;
	}

#if 0
	obj->thread = kthread_run(spr5801_thread_func, obj, "spr5801_thread");
	if (IS_ERR(obj->thread)) {
		pr_err("kthread_run() failed\n");
		return PTR_ERR(obj->thread);
	}
#endif

	spr5801_reset(obj);
	//spr5801_read_chip_id(obj);

	/* XXX: fail abruptly, so that i2cdetect can be used further */
	return -1;
	//return 0;
}

static int spr5801_remove(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct spr5801 *obj = dev_get_drvdata(dev);

	if (obj->thread)
		kthread_stop(obj->thread);

	return 0;
}

static const struct i2c_device_id spr5801_id[] = {
	{ "spr5801" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, spr5801_id);

static const struct of_device_id spr5801_of_match[] = {
	{ .compatible = "gyrfalcon,spr5801s12" },
	{ }
};
MODULE_DEVICE_TABLE(of, spr5801_of_match);

static struct i2c_driver spr5801_driver = {
	.driver = {
		.name           = "spr5801",
		.of_match_table = spr5801_of_match,
	},
	.probe          = spr5801_probe,
	.remove         = spr5801_remove,
	.id_table       = spr5801_id,
};

module_i2c_driver(spr5801_driver);
