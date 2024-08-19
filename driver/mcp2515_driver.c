#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>

// mcp2515一些寄存器地址和指令的宏定义
/******************************/
#define MCP2515_RESET 0xc0
#define MCP2515_READ 0x03
#define MCP2515_WRITE 0x02
#define MCP2515_CHANGE_REGBIT 0x05

#define MCP2515_STATUS 0x0e
#define MCP2515_CNF1 0x2a
#define MCP2515_CNF2 0x29
#define MCP2515_CNF3 0x28
#define MCP2515_CANINTE 0x2b
#define MCP2515_CANCTRL 0x0f
#define MCP2515_TXB0CTRL 0x30
#define MCP2515_CANINTF 0x2c
#define MCP2515_RXB0CTRL 0x60
/*****************************/

int flag1 = 1;
int flag2 = 1;
int flag3 = 1;
int flag4 = 1;
int flag5 = 1;
dev_t dev_num;
struct cdev mcp2515_cdev;
struct class *mcp2515_class;
struct device *mcp2515_device;
struct spi_device *spi_dev;

void mcp2515_reset(void)
{
    int ret;
    u8 write_buf[1] = {MCP2515_RESET};

    ret = spi_write(spi_dev, write_buf, sizeof(write_buf));
    if (ret < 0)
    {
        printk("spi_write_error_1\n");
    }
}

u8 mcp2515_read_reg(u8 reg_addr)
{
    u8 data;
    u8 write_buf[2] = {MCP2515_READ, reg_addr};
    int ret;
    ret = spi_write_then_read(spi_dev, write_buf, sizeof(write_buf), &data, sizeof(data));
    if (ret < 0)
    {
        printk("spi_write_then_read_error_1\n");
    }
    return data;
}

void mcp2515_write_reg(u8 reg_addr, u8 data)
{
    int ret;
    u8 write_buf[3] = {MCP2515_WRITE, reg_addr, data};

    ret = spi_write(spi_dev, write_buf, sizeof(write_buf));
    if (ret < 0)
    {
        printk("spi_write_error_2\n");
    }
}

void mcp2515_change_regbit(u8 reg, u8 mask, u8 value)
{
    int ret;
    u8 write_buf[4] = {MCP2515_CHANGE_REGBIT, reg, mask, value};

    ret = spi_write(spi_dev, write_buf, sizeof(write_buf));
    if (ret < 0)
    {
        printk("spi_write_error_3\n");
    }
}

int mcp2515_open(struct inode *inode, struct file *file)
{
    return 0;
}

ssize_t mcp2515_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    u8 read_buf[13];
    int ret;
    int i;
    while (!(mcp2515_read_reg(MCP2515_CANINTF) && (1 << 0)))
        ;
    for (i = 0; i < 13; i++)
    {
        read_buf[i] = mcp2515_read_reg(MCP2515_RXB0CTRL + 1 + i);
    }
    mcp2515_change_regbit(MCP2515_CANINTF, 0x01, 0x00);
    ret = copy_to_user(buf, read_buf, size);
    return size;
}

ssize_t mcp2515_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    u8 write_buf[13] = {0};
    u8 value;
    int ret;
    int i;
    value = mcp2515_read_reg(MCP2515_STATUS);
    printk("STATUS:%x\n", value);
    ret = copy_from_user(write_buf, buf, size);
    mcp2515_change_regbit(MCP2515_TXB0CTRL, 0x03, 0x03);
    for (i = 0; i < 13; i++)
    {
        mcp2515_write_reg(MCP2515_TXB0CTRL + 1 + i, write_buf[i]);
    }
    mcp2515_change_regbit(MCP2515_TXB0CTRL, 0x08, 0x08);
    while (!(mcp2515_read_reg(MCP2515_CANINTF) && (1 << 2)))
        ;
    mcp2515_change_regbit(MCP2515_CANINTF, 0x04, 0x00);
    value = mcp2515_read_reg(MCP2515_STATUS);
    printk("STATUS:%x\n", value);
    return size;
}

int mcp2515_release(struct inode *inode, struct file *file)
{
    return 0;
}

struct file_operations mcp2515_fops = {
    .open = mcp2515_open,
    .read = mcp2515_read,
    .write = mcp2515_write,
    .release = mcp2515_release,
};

int myspi_driver_probe(struct spi_device *spi)
{
    u8 value;
    spi_dev = spi;
    flag2 = alloc_chrdev_region(&dev_num, 0, 1, "mcp2515");
    if (flag2 < 0)
    {
        return -1;
    }

    cdev_init(&mcp2515_cdev, &mcp2515_fops);

    mcp2515_cdev.owner = THIS_MODULE;

    flag3 = cdev_add(&mcp2515_cdev, dev_num, 1);
    if (flag3 < 0)
    {
        return -1;
    }

    mcp2515_class = class_create(THIS_MODULE, "spi_can");
    if (IS_ERR(mcp2515_class))
    {
        flag4 = -1;
        return PTR_ERR(mcp2515_class);
    }

    mcp2515_device = device_create(mcp2515_class, NULL, dev_num, NULL, "mcp2515");
    if (IS_ERR(mcp2515_device))
    {
        flag5 = -1;
        return PTR_ERR(mcp2515_device);
    }

    value = mcp2515_read_reg(MCP2515_STATUS);
    printk("STATUS:%x\n", value);
    mcp2515_reset();
    mcp2515_write_reg(MCP2515_CNF1, 0x01);
    mcp2515_write_reg(MCP2515_CNF2, 0xb1);
    mcp2515_write_reg(MCP2515_CNF3, 0x05);
    mcp2515_change_regbit(MCP2515_RXB0CTRL, 0x64, 0x64);
    mcp2515_write_reg(MCP2515_CANINTE, 0x05);
    mcp2515_change_regbit(MCP2515_CANCTRL, 0xe0, 0x40);
    value = mcp2515_read_reg(MCP2515_STATUS);
    printk("STATUS:%x\n", value);

    return 0;
}

int myspi_driver_remove(struct spi_device *spi)
{
    return 0;
}

const struct of_device_id myspi_mcp2515_id[] = {
    {.compatible = "myspi-mcp2515"},
    {},
};

struct spi_driver myspi_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "myspi_driver_mcp2515",
        .of_match_table = myspi_mcp2515_id,
    },
    .probe = myspi_driver_probe,
    .remove = myspi_driver_remove,
};

static int __init myspi_driver_init(void)
{
    flag1 = spi_register_driver(&myspi_driver);
    if (flag1 < 0)
    {
        printk("add error\n");
        return -1;
    }
    return 0;
}

static void __exit myspi_driver_exit(void)
{
    if (flag5 >= 0)
    {
        device_destroy(mcp2515_class, dev_num);
    }
    if (flag4 >= 0)
    {
        class_destroy(mcp2515_class);
    }
    if (flag3 >= 0)
    {
        cdev_del(&mcp2515_cdev);
    }
    if (flag2 >= 0)
    {
        unregister_chrdev_region(dev_num, 1);
    }
    if (flag1 >= 0)
    {
        spi_unregister_driver(&myspi_driver);
    }
}

module_init(myspi_driver_init);
module_exit(myspi_driver_exit);

MODULE_LICENSE("GPL");
