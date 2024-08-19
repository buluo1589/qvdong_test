#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>

// mpu6050一些寄存器地址的宏定义
/******************************/
#define WHO 0x75 // mpu6050 who am i 寄存器地址
#define AX_H 0x3B
#define AX_L 0x3C
#define AY_H 0x3D
#define AY_L 0x3E
#define AZ_H 0x3F
#define AZ_L 0x40
#define TEMP_H 0x41
#define TEMP_L 0x42
#define GX_H 0x43
#define GX_L 0x44
#define GY_H 0x45
#define GY_L 0x46
#define GZ_H 0x47
#define GZ_L 0x48
/*****************************/

int flag1 = 1;
int flag2 = 1;
int flag3 = 1;
int flag4 = 1;
int flag5 = 1;
dev_t dev_num;
struct i2c_client *myi2c_client;
struct cdev mpu6050_cdev;
struct class *mpu6050_class;
struct device *mpu6050_device;

u8 myi2c_read_reg(u8 reg_addr)
{
    u8 data;
    struct i2c_msg msgs_r[] = {
        [0] = {
            .addr = myi2c_client->addr,
            .flags = 0,
            .len = sizeof(reg_addr),
            .buf = &reg_addr,
        },
        [1] = {
            .addr = myi2c_client->addr,
            .flags = 1,
            .len = sizeof(data),
            .buf = &data,
        },
    };
    i2c_transfer(myi2c_client->adapter, msgs_r, 2);
    return data;
}

void myi2c_write_reg(u8 reg_addr, u8 data)
{
    u8 buf[2];
    buf[0] = reg_addr;
    buf[1] = data;

    { //  for warning: ISO C90 forbids mixed declarations and code
        struct i2c_msg msgs_w[] = {
            [0] = {
                .addr = myi2c_client->addr,
                .flags = 0,
                .len = sizeof(buf),
                .buf = buf,
            },
        };
        i2c_transfer(myi2c_client->adapter, msgs_w, 1);
    }
}

int mpu6050_open(struct inode *inode, struct file *file)
{
    u8 value = myi2c_read_reg(WHO);
    if (value != 0x68)
    {
        printk("WHO not 0x68\n");
        return -1;
    }
    return 0;
}

ssize_t mpu6050_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    int ret, i;
    u8 value[14] = {0};
    for (i = 0; i < 14; i++)
    {

        value[i] = myi2c_read_reg(AX_H + i);
    }
    ret = copy_to_user(buf, value, size);
    if (ret < 0)
    {
        printk("read: copy_error\n");
        return -1;
    }
    return size;
}

ssize_t mpu6050_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    int ret;
    u8 u_buf[2] = {0};
    ret = copy_from_user(u_buf, buf, size);
    if (ret < 0)
    {
        printk("write: copy_error\n");
        return -1;
    }
    myi2c_write_reg(u_buf[0], u_buf[1]);
    return size;
}

int mpu6050_release(struct inode *inode, struct file *file)
{
    return 0;
}

struct file_operations mpu6050_fops = {
    .open = mpu6050_open,
    .read = mpu6050_read,
    .write = mpu6050_write,
    .release = mpu6050_release,
};

int myi2c_driver_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    myi2c_client = client;

    flag2 = alloc_chrdev_region(&dev_num, 0, 1, "mpu6050");
    if (flag2 < 0)
    {
        return -1;
    }

    cdev_init(&mpu6050_cdev, &mpu6050_fops);

    mpu6050_cdev.owner = THIS_MODULE;

    flag3 = cdev_add(&mpu6050_cdev, dev_num, 1);
    if (flag3 < 0)
    {
        return -1;
    }

    mpu6050_class = class_create(THIS_MODULE, "get_a_g");
    if (IS_ERR(mpu6050_class))
    {
        flag4 = -1;
        return PTR_ERR(mpu6050_class);
    }

    mpu6050_device = device_create(mpu6050_class, NULL, dev_num, NULL, "mpu6050");
    if (IS_ERR(mpu6050_device))
    {
        flag5 = -1;
        return PTR_ERR(mpu6050_device);
    }
    return 0;
}

int myi2c_driver_remove(struct i2c_client *client)
{
    return 0;
}

const struct of_device_id myi2c_mpu6050_id[] = {
    {.compatible = "myi2c-mpu6050"},
    {},
};

struct i2c_driver myi2c_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "myi2c_driver_mpu6050",
        .of_match_table = myi2c_mpu6050_id,
    },
    .probe = myi2c_driver_probe,
    .remove = myi2c_driver_remove,
};

static int __init myi2c_driver_init(void)
{
    flag1 = i2c_add_driver(&myi2c_driver);

    if (flag1 < 0)
    {
        printk("add error\n");
        return -1;
    }
    return 0;
}

static void __exit myi2c_driver_exit(void)
{
    if (flag5 >= 0)
    {
        device_destroy(mpu6050_class, dev_num);
    }
    if (flag4 >= 0)
    {
        class_destroy(mpu6050_class);
    }
    if (flag3 >= 0)
    {
        cdev_del(&mpu6050_cdev);
    }
    if (flag2 >= 0)
    {
        unregister_chrdev_region(dev_num, 1);
    }
    if (flag1 >= 0)
    {
        i2c_del_driver(&myi2c_driver);
    }
}

module_init(myi2c_driver_init);
module_exit(myi2c_driver_exit);

MODULE_LICENSE("GPL");
