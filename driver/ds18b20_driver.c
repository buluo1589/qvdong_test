#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>

#define SET_RESOLUTION _IOW('A', 0, int)
#define READ_RESOLUTION _IOR('A', 1, int)
#define SET_RESOLUTION_9 9
#define SET_RESOLUTION_10 10
#define SET_RESOLUTION_11 11
#define SET_RESOLUTION_12 12

int flag1 = 1;
int flag2 = 1;
int flag3 = 1;
int flag4 = 1;
int flag5 = 1;
dev_t dev_num;
struct cdev ds18b20_cdev;
struct class *ds18b20_class;
struct device *ds18b20_device;
struct gpio_desc *ds18b20_gpio;

void ds18b20_reset(void)
{
    gpiod_direction_output(ds18b20_gpio, 1);
    gpiod_set_value(ds18b20_gpio, 0);
    udelay(600);

    gpiod_set_value(ds18b20_gpio, 1); // 需要延时15-20us，据说这一步时间差不多15-20us，所以不加延时

    gpiod_direction_input(ds18b20_gpio);
    while (gpiod_get_value(ds18b20_gpio))
        ; // 等待拉低
    while (!gpiod_get_value(ds18b20_gpio))
        ; // 等待拉高
    udelay(500);
}

void ds18b20_write_bit(unsigned char bit)
{
    gpiod_direction_output(ds18b20_gpio, 1);
    gpiod_set_value(ds18b20_gpio, 0);
    if (bit)
    {
        udelay(10);
        gpiod_direction_output(ds18b20_gpio, 1);
    }
    udelay(65);

    gpiod_direction_output(ds18b20_gpio, 1);
    udelay(2);
}

void ds18b20_write_byte(int data)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        ds18b20_write_bit(data & 0x01);
        data = data >> 1;
    }
}

unsigned char ds18b20_read_bit(void)
{
    unsigned char bit;
    gpiod_direction_output(ds18b20_gpio, 1);
    gpiod_set_value(ds18b20_gpio, 0);
    udelay(2);

    gpiod_direction_input(ds18b20_gpio);
    udelay(10);
    bit = gpiod_get_value(ds18b20_gpio);
    udelay(60);

    return bit;
}

int ds18b20_read_byte(void)
{
    int i;
    int data;
    for (i = 0; i < 8; i++)
    {
        data |= ds18b20_read_bit() << i;
    }
    return data;
}

int ds18b20_read_temp(void)
{
    int temp_l, temp_h, temp;

    ds18b20_reset();
    ds18b20_write_byte(0xcc);
    ds18b20_write_byte(0x44);
    mdelay(750);
    ds18b20_reset();
    ds18b20_write_byte(0xcc);
    ds18b20_write_byte(0xbe);

    temp_l = ds18b20_read_byte();
    temp_h = ds18b20_read_byte();

    temp = temp_h << 8 | temp_l;

    return temp;
}

void ds18b20_init(int args)
{
    ds18b20_reset();
    ds18b20_write_byte(0xcc);
    ds18b20_write_byte(0x4e);

    ds18b20_write_byte(60); // 上限报警
    ds18b20_write_byte(10); // 下限报警

    switch (args)
    {
    case SET_RESOLUTION_9:
        ds18b20_write_byte(0x1f);
        break;
    case SET_RESOLUTION_10:
        ds18b20_write_byte(0x3f);
        break;
    case SET_RESOLUTION_11:
        ds18b20_write_byte(0x5f);
        break;
    case SET_RESOLUTION_12:
        ds18b20_write_byte(0x7f);
        break;
    default:
        break;
    }
}

int ds18b20_read_resolution(void)
{
    ds18b20_reset();
    ds18b20_write_byte(0xcc);
    ds18b20_write_byte(0xbe);

    printf("%d\n", ds18b20_read_byte());
    printf("%d\n", ds18b20_read_byte());
    printf("%d\n", ds18b20_read_byte());
    printf("%d\n", ds18b20_read_byte());
    // ds18b20_read_byte();
    // ds18b20_read_byte();
    // ds18b20_read_byte();
    // ds18b20_read_byte();

    switch (ds18b20_read_byte())
    {
    case 0x1f:
        return 9;
    case 0x3f:
        return 10;
    case 0x5f:
        return 11;
    case 0x7f:
        return 12;
    default:
        return -1;
    }
}

int ds18b20_open(struct inode *inode, struct file *file)
{
    return 0;
}

ssize_t ds18b20_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    int temp, ret;

    temp = ds18b20_read_temp();

    ret = copy_to_user(buf, &temp, sizeof(temp));
    if (ret < 0)
    {
        return -1;
    }

    return size;
}

ssize_t ds18b20_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    return size;
}

int ds18b20_release(struct inode *inode, struct file *file)
{
    return 0;
}

long ds18b20_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
    int resolution;
    if (cmd == SET_RESOLUTION)
    {
        if (args >= SET_RESOLUTION_9 && args <= SET_RESOLUTION_12)
        {
            printk("set_resolution:%ld\n", args);
            ds18b20_init(args);
        }
    }
    else if (cmd == READ_RESOLUTION)
    {
        int ret;
        resolution = ds18b20_read_resolution();
        printk("resolution:%d\n", resolution);
        // ret = copy_to_user((int __user *)args, &resolution, sizeof(resolution));
        ret = __put_user(resolution, (int __user *)args);
    }
    return 0;
}

struct file_operations ds18b20_fops = {
    .open = ds18b20_open,
    .read = ds18b20_read,
    .write = ds18b20_write,
    .release = ds18b20_release,
    .unlocked_ioctl = ds18b20_ioctl,
};

int ds18b20_driver_probe(struct platform_device *dev)
{
    flag2 = alloc_chrdev_region(&dev_num, 0, 1, "ds18b20");
    if (flag2 < 0)
    {
        return -1;
    }

    cdev_init(&ds18b20_cdev, &ds18b20_fops);

    ds18b20_cdev.owner = THIS_MODULE;

    flag3 = cdev_add(&ds18b20_cdev, dev_num, 1);
    if (flag3 < 0)
    {
        return -1;
    }

    ds18b20_class = class_create(THIS_MODULE, "sensors");
    if (IS_ERR(ds18b20_class))
    {
        flag4 = -1;
        return PTR_ERR(ds18b20_class);
    }

    ds18b20_device = device_create(ds18b20_class, NULL, dev_num, NULL, "ds18b20");
    if (IS_ERR(ds18b20_device))
    {
        flag5 = -1;
        return PTR_ERR(ds18b20_device);
    }

    ds18b20_gpio = gpiod_get_optional(&dev->dev, "ds18b20", 0);
    if (ds18b20_gpio == NULL)
    {
        return -1;
    }

    gpiod_direction_output(ds18b20_gpio, 1);

    return 0;
}

int ds18b20_driver_remove(struct platform_device *dev)
{
    return 0;
}

const struct of_device_id ds18b20_id[] = {
    {.compatible = "ds18b20"},
    {},
};

struct platform_driver ds18b20_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "ds18b20_driver",
        .of_match_table = ds18b20_id,
    },
    .probe = ds18b20_driver_probe,
    .remove = ds18b20_driver_remove,
};

static int __init ds18b20_driver_init(void)
{
    flag1 = platform_driver_register(&ds18b20_driver);

    if (flag1 < 0)
    {
        printk("add error\n");
        return -1;
    }
    return 0;
}

static void __exit ds18b20_driver_exit(void)
{
    if (flag5 >= 0)
    {
        device_destroy(ds18b20_class, dev_num);
    }
    if (flag4 >= 0)
    {
        class_destroy(ds18b20_class);
    }
    if (flag3 >= 0)
    {
        cdev_del(&ds18b20_cdev);
    }
    if (flag2 >= 0)
    {
        unregister_chrdev_region(dev_num, 1);
    }
    if (flag1 >= 0)
    {
        platform_driver_unregister(&ds18b20_driver);
    }
}

module_init(ds18b20_driver_init);
module_exit(ds18b20_driver_exit);

MODULE_LICENSE("GPL");
