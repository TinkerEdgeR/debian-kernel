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
static const char *adc5_bid;

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
	struct proc_dir_entry* file;

	boardinfo = "Tinker Edge R";

	if (device_property_read_string(dev, "boardver", &adc5_bid)) {
		printk("[boardinfo] Failed to read boardver\n");
		return -ENODEV;
	}

	if (strcmp(adc5_bid, "-1") == 0) {
		printk("[uboot/adc] Failed to read channel raw\n");
		return -ENODEV;
	} else if (strcmp(adc5_bid, "0") == 0)
		boardver = "unknown";
	else if (strcmp(adc5_bid, "1") == 0)
		boardver = "1.01";
	else if (strcmp(adc5_bid, "2") == 0)
		boardver = "1.02";
	else if (strcmp(adc5_bid, "3") == 0)
		boardver = "1.03";
	else if (strcmp(adc5_bid, "4") == 0)
		boardver = "1.04";

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

int boardver_show(void)
{
	int bid;

	if (strcmp(adc5_bid, "-1") == 0)
		return -ENODEV;
	else {
		bid = adc5_bid[0] - '0';
		return bid;
	}
}
EXPORT_SYMBOL_GPL(boardver_show);

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
