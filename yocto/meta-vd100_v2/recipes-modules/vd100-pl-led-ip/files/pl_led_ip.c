// SPDX-License-Identifier: GPL-2.0
/*
 * plledip.c — VD100 PL LED + PL Reset IRQ + PS_KEY Driver
 *
 * Manages PL_LED1 via AXI-lite plledip peripheral (0xA4000000).
 * Detects PL fabric reset via pl_ps_irq[0] (GIC SPI 84, level-high).
 * Handles PS_KEY GPIO interrupt for interactive LED toggle.
 * Exposes /dev/plledip character device and sysfs pl_led_state attribute.
 *
 * Hardware mapping (VD100 XCVE2302):
 *   PL_LED1  — E20, LVCMOS15, active-low  — AXI plledip offset 0x00
 *   PL_KEY1  — F21, LVCMOS15, active-low  — aux_reset_in + pl_ps_irq[0]
 *   PS_KEY   — LPD MIO24, active-low      — GPIO interrupt
 *   PS_LED1  — LPD MIO25, active-low      — GPIO output
 *
 * PL LED semantic: ON = PL fabric running, OFF = PL in reset.
 * Shadow register is source of truth in driver; hardware register
 * is source of truth on probe (survives PS reboot).
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/sysfs.h>
#include <linux/device.h>

#define DRIVER_NAME             "plledip"
#define PLLEDIP_CTRL_REG        0x00    /* slv_reg0 — LED control, bit 0 */
#define PLLEDIP_STATUS_REG      0x04    /* slv_reg1 — status readback    */
#define PL_RESET_RESTORE_MS     200     /* wait after PL reset before AXI access */

/* ── Driver private data ──────────────────────────────────────────────────── */

struct plledip_dev {
	void __iomem		*base;          /* AXI-lite register base          */
	struct device		*dev;
	int			 irq_pl;        /* pl_ps_irq[0] — GIC SPI 84       */
	struct gpio_desc	*ps_key_gpio;   /* LPD MIO24 PS_KEY, active-low    */
	struct gpio_desc	*ps_led_gpio;   /* LPD MIO25 PS_LED, active-low    */
	u32			 led_shadow;    /* last-written LED state 0=OFF 1=ON */
	spinlock_t		 lock;
	struct delayed_work	 pl_restore;    /* post-reset LED restore work     */
	struct cdev		 cdev;
	dev_t			 devno;
	struct class		*cls;
};

/* ── AXI register helpers ─────────────────────────────────────────────────── */

/*
 * plledip_hw_set — write LED state to hardware and update shadow.
 * Must be called with mdev->lock held when invoked from IRQ context.
 */
static void plledip_hw_set(struct plledip_dev *mdev, u32 val)
{
	mdev->led_shadow = val ? 1 : 0;
	iowrite32(mdev->led_shadow, mdev->base + PLLEDIP_CTRL_REG);
}

/*
 * plledip_hw_get — read LED state directly from hardware register.
 * Used only during probe to reconstruct shadow after PS reboot.
 */
static u32 plledip_hw_get(struct plledip_dev *mdev)
{
	return ioread32(mdev->base + PLLEDIP_CTRL_REG) & 0x1;
}

/* ── Sysfs ────────────────────────────────────────────────────────────────── */

static ssize_t pl_led_state_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct plledip_dev *mdev = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", mdev->led_shadow);
}

static ssize_t pl_led_state_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct plledip_dev *mdev = dev_get_drvdata(dev);
	unsigned long flags;
	u32 val;

	if (kstrtou32(buf, 10, &val) || val > 1)
		return -EINVAL;

	spin_lock_irqsave(&mdev->lock, flags);
	plledip_hw_set(mdev, val);
	if (mdev->ps_led_gpio)
		gpiod_set_value(mdev->ps_led_gpio, val);
	spin_unlock_irqrestore(&mdev->lock, flags);

	sysfs_notify(&mdev->dev->kobj, NULL, "pl_led_state");
	return count;
}

/*
 * mode 0664 — root:pldev rw-rw-r--
 * pldev group created via EXTRA_USERS_PARAMS in vd100.conf
 */
static struct device_attribute dev_attr_pl_led_state = {
	.attr  = { .name = "pl_led_state", .mode = 0664 },
	.show  = pl_led_state_show,
	.store = pl_led_state_store,
};

/* ── Delayed work — post PL reset restore ────────────────────────────────── */

static void pl_restore_work(struct work_struct *work)
{
	struct plledip_dev *mdev =
		container_of(work, struct plledip_dev, pl_restore.work);
	unsigned long flags;

	/*
	 * PL has exited reset — AXI bus is accessible again.
	 * Restore LED ON to indicate PL fabric is running.
	 */
	spin_lock_irqsave(&mdev->lock, flags);
	plledip_hw_set(mdev, 1);
	if (mdev->ps_led_gpio)
		gpiod_set_value(mdev->ps_led_gpio, 1);
	spin_unlock_irqrestore(&mdev->lock, flags);

	dev_info(mdev->dev, "PL restart complete — LED restored ON\n");
	sysfs_notify(&mdev->dev->kobj, NULL, "pl_led_state");
}

/* ── pl_ps_irq[0] ISR — PL fabric reset interrupt ───────────────────────── */

/*
 * Fires when PL_KEY1 is pressed (active-low → NOT gate → active-high to CIPS).
 * At this point the PL peripheral is entering reset — do NOT access AXI
 * registers here. Update shadow only and schedule delayed restore.
 */
static irqreturn_t pl_reset_irq_handler(int irq, void *data)
{
	struct plledip_dev *mdev = data;

	dev_warn(mdev->dev, "PL is restarting — LED was %s\n",
		 mdev->led_shadow ? "ON" : "OFF");

	/*
	 * Hardware reset clears slv_reg0. Update shadow immediately
	 * so sysfs reflects reality without any AXI read.
	 */
	mdev->led_shadow = 0;
	sysfs_notify(&mdev->dev->kobj, NULL, "pl_led_state");

	/* Restore LED after PL exits reset */
	schedule_delayed_work(&mdev->pl_restore,
			      msecs_to_jiffies(PL_RESET_RESTORE_MS));

	return IRQ_HANDLED;
}

/* ── PS_KEY GPIO IRQ — interactive LED toggle ────────────────────────────── */

/*
 * Threaded IRQ — safe to call gpiod_set_value and sysfs_notify here.
 * IRQF_TRIGGER_FALLING: active-low button, press = falling edge.
 * Shadow is always valid — no file read required.
 */
static irqreturn_t ps_key_irq_handler(int irq, void *data)
{
	struct plledip_dev *mdev = data;
	unsigned long flags;
	u32 new_state;

	spin_lock_irqsave(&mdev->lock, flags);
	new_state = mdev->led_shadow ^ 1;
	plledip_hw_set(mdev, new_state);
	if (mdev->ps_led_gpio)
		gpiod_set_value(mdev->ps_led_gpio, new_state);
	spin_unlock_irqrestore(&mdev->lock, flags);

	dev_info(mdev->dev, "PS_KEY: LEDs toggled → %u\n", new_state);
	sysfs_notify(&mdev->dev->kobj, NULL, "pl_led_state");

	return IRQ_HANDLED;
}

/* ── Character device — /dev/plledip ─────────────────────────────────────── */

static int plledip_open(struct inode *inode, struct file *filp)
{
	filp->private_data = container_of(inode->i_cdev,
					   struct plledip_dev, cdev);
	return 0;
}

/*
 * Read: returns current LED shadow state as ASCII "0\n" or "1\n".
 * Shadow is always coherent with hardware (probe reconstructs it).
 */
static ssize_t plledip_read(struct file *filp, char __user *ubuf,
			     size_t count, loff_t *ppos)
{
	struct plledip_dev *mdev = filp->private_data;
	char kbuf[4];
	int len;

	len = scnprintf(kbuf, sizeof(kbuf), "%u\n", mdev->led_shadow);
	return simple_read_from_buffer(ubuf, count, ppos, kbuf, len);
}

/*
 * Write: accepts "0" or "1" to control LED.
 * Updates shadow, hardware register, PS_LED, and notifies sysfs.
 * Existing userspace (echo 1 | tee /dev/plledip) continues to work.
 */
static ssize_t plledip_write(struct file *filp, const char __user *ubuf,
			      size_t count, loff_t *ppos)
{
	struct plledip_dev *mdev = filp->private_data;
	unsigned long flags;
	char kbuf[4] = { 0 };
	u32 val;

	if (count > sizeof(kbuf) - 1)
		return -EINVAL;
	if (copy_from_user(kbuf, ubuf, count))
		return -EFAULT;
	if (kstrtou32(kbuf, 10, &val) || val > 1)
		return -EINVAL;

	spin_lock_irqsave(&mdev->lock, flags);
	plledip_hw_set(mdev, val);
	if (mdev->ps_led_gpio)
		gpiod_set_value(mdev->ps_led_gpio, val);
	spin_unlock_irqrestore(&mdev->lock, flags);

	sysfs_notify(&mdev->dev->kobj, NULL, "pl_led_state");
	return count;
}

static const struct file_operations plledip_fops = {
	.owner	= THIS_MODULE,
	.open	= plledip_open,
	.read	= plledip_read,
	.write	= plledip_write,
};

/* ── Probe ────────────────────────────────────────────────────────────────── */

static int plledip_probe(struct platform_device *pdev)
{
	struct plledip_dev *mdev;
	int irq, ret;

	mdev = devm_kzalloc(&pdev->dev, sizeof(*mdev), GFP_KERNEL);
	if (!mdev)
		return -ENOMEM;

	mdev->dev = &pdev->dev;
	spin_lock_init(&mdev->lock);
	INIT_DELAYED_WORK(&mdev->pl_restore, pl_restore_work);
	platform_set_drvdata(pdev, mdev);

	/* ── AXI register mapping ── */
	mdev->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(mdev->base))
		return PTR_ERR(mdev->base);

	/*
	 * Reconstruct shadow from hardware — source of truth on probe.
	 * Handles PS reboot: /var/run is gone but hardware state persists
	 * as long as PL power was not cycled.
	 */
	mdev->led_shadow = plledip_hw_get(mdev);
	dev_info(&pdev->dev, "PL LED state on probe: %u\n", mdev->led_shadow);

	/* PL LED ON = PL fabric is running */
	plledip_hw_set(mdev, 1);
	dev_info(&pdev->dev, "PL running — LED ON\n");

	/* ── pl_ps_irq[0] — PL reset interrupt (GIC SPI 84) ── */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "pl_ps_irq not found in DTS — check interrupts property\n");
		return irq;
	}
	mdev->irq_pl = irq;

	ret = devm_request_irq(&pdev->dev, irq, pl_reset_irq_handler,
			       IRQF_TRIGGER_RISING, "plledip_pl_reset", mdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to request pl_ps_irq %d: %d\n", irq, ret);
		return ret;
	}
	dev_info(&pdev->dev, "pl_ps_irq registered: IRQ %d\n", irq);

	/* ── PS_KEY GPIO — LPD MIO24, active-low ── */
	mdev->ps_key_gpio = devm_gpiod_get_optional(&pdev->dev, "ps-key", GPIOD_IN);
	if (IS_ERR(mdev->ps_key_gpio)) {
		dev_warn(&pdev->dev, "ps-key GPIO unavailable — PS_KEY disabled\n");
		mdev->ps_key_gpio = NULL;
	} else if (mdev->ps_key_gpio) {
		irq = gpiod_to_irq(mdev->ps_key_gpio);
		if (irq < 0) {
			dev_warn(&pdev->dev, "ps-key gpiod_to_irq failed: %d\n", irq);
		} else {
			ret = devm_request_threaded_irq(&pdev->dev, irq,
							NULL,
							ps_key_irq_handler,
							IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
							"plledip_ps_key", mdev);
			if (ret)
				dev_warn(&pdev->dev, "ps-key IRQ request failed: %d\n", ret);
			else
				dev_info(&pdev->dev, "PS_KEY registered: IRQ %d\n", irq);
		}
	}

	/* ── PS_LED GPIO — LPD MIO25, active-low ── */
	mdev->ps_led_gpio = devm_gpiod_get_optional(&pdev->dev, "ps-led", GPIOD_OUT_LOW);
	if (IS_ERR(mdev->ps_led_gpio)) {
		dev_warn(&pdev->dev, "ps-led GPIO unavailable — PS_LED disabled\n");
		mdev->ps_led_gpio = NULL;
	} else if (mdev->ps_led_gpio) {
		/* Mirror initial PL LED state to PS LED */
		gpiod_set_value(mdev->ps_led_gpio, mdev->led_shadow);
		dev_info(&pdev->dev, "PS_LED registered, state: %u\n", mdev->led_shadow);
	}

	/* ── Character device ── */
	ret = alloc_chrdev_region(&mdev->devno, 0, 1, DRIVER_NAME);
	if (ret) {
		dev_err(&pdev->dev, "alloc_chrdev_region failed: %d\n", ret);
		return ret;
	}

	/* kernel 6.4+: class_create takes only the name */
	mdev->cls = class_create(DRIVER_NAME);
	if (IS_ERR(mdev->cls)) {
		ret = PTR_ERR(mdev->cls);
		goto err_class;
	}

	cdev_init(&mdev->cdev, &plledip_fops);
	mdev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&mdev->cdev, mdev->devno, 1);
	if (ret) {
		dev_err(&pdev->dev, "cdev_add failed: %d\n", ret);
		goto err_cdev;
	}

	/* Creates /dev/plledip — udev rule applies GROUP=pldev MODE=0660 */
	if (!device_create(mdev->cls, &pdev->dev, mdev->devno, mdev, DRIVER_NAME)) {
		ret = -EINVAL;
		goto err_device;
	}

	/* ── sysfs attribute ── */
	ret = device_create_file(&pdev->dev, &dev_attr_pl_led_state);
	if (ret) {
		dev_warn(&pdev->dev, "sysfs pl_led_state creation failed: %d\n", ret);
		/* non-fatal — /dev/plledip still works */
	}

	dev_info(&pdev->dev,
		 "plledip probe complete — /dev/plledip ready, pl_ps_irq=%d\n",
		 mdev->irq_pl);
	return 0;

err_device:
	cdev_del(&mdev->cdev);
err_cdev:
	class_destroy(mdev->cls);
err_class:
	unregister_chrdev_region(mdev->devno, 1);
	return ret;
}

static void plledip_remove(struct platform_device *pdev)
{
	struct plledip_dev *mdev = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&mdev->pl_restore);
	device_remove_file(&pdev->dev, &dev_attr_pl_led_state);
	device_destroy(mdev->cls, mdev->devno);
	cdev_del(&mdev->cdev);
	class_destroy(mdev->cls);
	unregister_chrdev_region(mdev->devno, 1);

	dev_info(&pdev->dev, "plledip removed\n");
}

/* ── Device tree match ────────────────────────────────────────────────────── */

static const struct of_device_id plledip_of_match[] = {
	{ .compatible = "xlnx,MyLEDIP-1.0" },
	{ }
};
MODULE_DEVICE_TABLE(of, plledip_of_match);

static struct platform_driver plledip_driver = {
	.probe	= plledip_probe,
	.remove	= plledip_remove,
	.driver	= {
		.name		= DRIVER_NAME,
		.of_match_table	= plledip_of_match,
	},
};

module_platform_driver(plledip_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adilson Dias");
MODULE_DESCRIPTION("VD100 PL LED + PL Reset IRQ + PS_KEY Driver");
MODULE_VERSION("2.0");
