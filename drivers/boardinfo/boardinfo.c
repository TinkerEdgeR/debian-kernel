#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/iio/consumer.h>
#include <linux/proc_fs.h>

static char *boardinfo;
static char *boardver;

static int info_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", boardinfo);
	return 0;
}

static int ver_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", boardver);
	return 0;
}

static int info_open(struct inode *inode, struct file *file)
{
	return single_open(file, info_show, NULL);
}

static int ver_open(struct inode *inode, struct file *file)
{
	return single_open(file, ver_show, NULL);
}

static struct file_operations boardinfo_ops = {
	.owner	= THIS_MODULE,
	.open	= info_open,
	.read	= seq_read,
};

static struct file_operations boardver_ops = {
	.owner	= THIS_MODULE,
	.open	= ver_open,
	.read	= seq_read,
};

static int boardinfo_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret, raw, vref, bits;
	int vresult;
	struct iio_channel *channels;
	const char *channel_name;
	struct proc_dir_entry* file;

	boardinfo = "Tinker Edge R";

	if (device_property_read_string(dev, "io-channel-names", &channel_name))
	{
		printk("[iio] Failed to read io-channel-names");
		return -ENODEV;
	}

	channels = devm_iio_channel_get(dev, channel_name);
	if (IS_ERR(channels)) {
		printk("[iio] Failed to get channels\n");
		return -ENODEV;
	} else {
		ret = iio_read_channel_raw(channels, &raw);
		if (ret < 0) {
			printk("[iio] Failed to read channel raw\n");
			return ret;
		}

		ret = iio_read_channel_scale(channels, &vref, &bits);
		if (ret < 0) {
			printk("[iio] Failed to read channel scale\n");
			return ret;
		}
	}

	vresult = vref * raw / ((2 << (bits - 1)) - 1);

	if (vresult < 1900 && vresult > 1700)
		boardver = "1.02";
	else if (vresult < 1300 && vresult > 1100)
		boardver = "1.04";
	else if (vresult < 1000 && vresult > 800)
		boardver = "1.01";
	else if (vresult < 100)
		boardver = "1.03";
	else
		boardver = "unknown";

	printk("boardinfo = %s\n", boardinfo);
	printk("boardver = %s\n", boardver);

	file = proc_create("boardinfo", 0444, NULL, &boardinfo_ops);
	if (!file)
		return -ENOMEM;

	file = proc_create("boardver", 0444, NULL, &boardver_ops);
	if (!file)
		return -ENOMEM;

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id of_boardinfo_match[] = {
	{ .compatible = "ADC5-BOARD-ID", },
	{ }
};
MODULE_DEVICE_TABLE(of, of_boardinfo_match);
#endif

static struct platform_driver boardinfo_driver = {
	.probe	= boardinfo_probe,
	.driver	= {
		.name	= "boardinfo",
		.of_match_table	= of_match_ptr(of_boardinfo_match),
	},
};
module_platform_driver(boardinfo_driver);
