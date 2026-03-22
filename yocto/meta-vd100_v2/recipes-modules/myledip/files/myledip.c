#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define DRIVER_NAME     "myledip"
#define LED_CTRL_REG    0x00
#define LED_STATUS_REG  0x04

#define MYLEDIP_S00_AXI_SLV_REG0_OFFSET 0
#define MYLEDIP_S00_AXI_SLV_REG1_OFFSET 4
#define MYLEDIP_S00_AXI_SLV_REG2_OFFSET 8
#define MYLEDIP_S00_AXI_SLV_REG3_OFFSET 12


struct myledip_dev {
    void __iomem    *base;
    struct cdev      cdev;
    dev_t            devno;
    struct class    *class;
    struct device   *device;
};

static struct myledip_dev *myledip;

/* Write '1' = LED on, '0' = LED off */
static ssize_t myledip_write(struct file *f, const char __user *buf,
                              size_t count, loff_t *off)
{
    char kbuf[2] = {0};

    if (count == 0)
        return -EINVAL;

    if (copy_from_user(kbuf, buf, min(count, sizeof(kbuf) - 1)))
        return -EFAULT;

    if (kbuf[0] == '1')
        iowrite32(0x1, myledip->base + LED_CTRL_REG);
    else
        iowrite32(0x0, myledip->base + LED_CTRL_REG);

    return count;
}

static ssize_t myledip_read(struct file *f, char __user *buf,
                             size_t count, loff_t *off)
{
    u32 val;
    char kbuf[2];

    if (*off > 0)
        return 0;

    val = ioread32(myledip->base + LED_STATUS_REG);
    kbuf[0] = (val & 0x1) ? '1' : '0';
    kbuf[1] = '\n';

    if (copy_to_user(buf, kbuf, 2))
        return -EFAULT;

    *off += 2;
    return 2;
}

static const struct file_operations myledip_fops = {
    .owner  = THIS_MODULE,
    .write  = myledip_write,
    .read   = myledip_read,
};

static int myledip_probe(struct platform_device *pdev)
{
    struct resource *res;
    int ret;

    myledip = devm_kzalloc(&pdev->dev, sizeof(*myledip), GFP_KERNEL);
    if (!myledip)
        return -ENOMEM;

    /* Map registers */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    myledip->base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(myledip->base))
        return PTR_ERR(myledip->base);

    /* Register character device */
    ret = alloc_chrdev_region(&myledip->devno, 0, 1, DRIVER_NAME);
    if (ret)
        return ret;

    cdev_init(&myledip->cdev, &myledip_fops);
    myledip->cdev.owner = THIS_MODULE;
    ret = cdev_add(&myledip->cdev, myledip->devno, 1);
    if (ret)
        goto err_chrdev;

    myledip->class = class_create(DRIVER_NAME);
    if (IS_ERR(myledip->class)) {
        ret = PTR_ERR(myledip->class);
        goto err_cdev;
    }

    myledip->device = device_create(myledip->class, NULL,
                                     myledip->devno, NULL,
                                     DRIVER_NAME);
    if (IS_ERR(myledip->device)) {
        ret = PTR_ERR(myledip->device);
        goto err_class;
    }

    dev_info(&pdev->dev, "MyLEDIP probed at 0x%llx → /dev/%s\n",
             (u64)res->start, DRIVER_NAME);
    return 0;

err_class:
    class_destroy(myledip->class);
err_cdev:
    cdev_del(&myledip->cdev);
err_chrdev:
    unregister_chrdev_region(myledip->devno, 1);
    return ret;
}

static void myledip_remove(struct platform_device *pdev)
{
    device_destroy(myledip->class, myledip->devno);
    class_destroy(myledip->class);
    cdev_del(&myledip->cdev);
    unregister_chrdev_region(myledip->devno, 1);
   // return 0;
}

static const struct of_device_id myledip_of_match[] = {
    { .compatible = "xlnx,MyLEDIP-1.0" },
    { }
};
MODULE_DEVICE_TABLE(of, myledip_of_match);

static struct platform_driver myledip_driver = {
    .probe  = myledip_probe,
    .remove = myledip_remove,
    .driver = {
        .name           = DRIVER_NAME,
        .of_match_table = myledip_of_match,
    },
};

module_platform_driver(myledip_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adilson Dias");
MODULE_DESCRIPTION("VD100 PL LED AXI-lite driver");
