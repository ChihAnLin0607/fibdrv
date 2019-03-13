#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Fibonacci engine driver");
MODULE_VERSION("0.1");

#define DEV_FIBONACCI_NAME "fibonacci"
#include "bigN.h"

#define FAST_FIBONACCI

static dev_t fib_dev = 0;
static struct cdev *fib_cdev;
static struct class *fib_class;
static DEFINE_MUTEX(fib_mutex);
static ktime_t multi_ktime = 0;
static ktime_t ktime;

#ifdef FAST_FIBONACCI
static void fib_sequence(long long k, struct BigN *buf)
{
    struct BigN fn = {0, 0}, fn_plus_1 = {0, 0}, tmp1 = {0, 0}, tmp2 = {0, 0};
    //    printk(KERN_INFO "k = %lld\n", k);
    if (!k) {
        buf->lower = 0;
        buf->upper = 0;
        return;
    } else if (k == 1 || k == 2) {
        buf->lower = 1;
        buf->upper = 0;
        return;
    }

    if (!(k % 2)) {
        // k = 2n
        // f(2n) = 2f(n+1)f(n) - f(n)^2
        fib_sequence(k >> 1, &fn);               // fn := f(n)
        fib_sequence((k >> 1) + 1, &fn_plus_1);  // fn_plus_1 := f(n+1)
        ktime = ktime_get();
        multiBigN(&tmp1, fn, fn);       // tmp := f(n)^2
        multiBigN(buf, fn_plus_1, fn);  // buf := f(n+1)*f(n)
        multi_ktime = ktime_add(ktime_sub(ktime_get(), ktime), multi_ktime);
        addBigN(buf, *buf, *buf);    // buf := 2f(n+1)f(n)
        minusBigN(buf, *buf, tmp1);  // buf := 2f(n+1)f(n) - f(n)^2
    } else {
        // k = 2n + 1
        // f(2n+1) = f(n+1)^2 + f(n)^2
        fib_sequence((k - 1) >> 1, &fn);                 // fn := f(n)
        fib_sequence((((k - 1) >> 1) + 1), &fn_plus_1);  // fn_plus_1 := f(n+1)
        ktime = ktime_get();
        multiBigN(&tmp1, fn_plus_1, fn_plus_1);  // tmp1 := f(n+1)^2
        multiBigN(&tmp2, fn, fn);                // tmp2 := f(n)^2
        multi_ktime = ktime_add(ktime_sub(ktime_get(), ktime), multi_ktime);
        addBigN(buf, tmp1, tmp2);  // buf := f(n+1)^2 + f(n)^2
    }
}

#else
static long long fib_sequence(long long k, char *buf, size_t size)
{
    struct BigN f[k + 2];
    memset(f, 0, sizeof(struct BigN) * (k + 2));

    f[0].lower = 0;
    f[1].lower = 1;

    for (int i = 2; i <= k; i++)
        addBigN(&f[i], f[i - 1], f[i - 2]);

    copy_to_user(buf, &f[k], size);
    return 1;
}
#endif

static int fib_open(struct inode *inode, struct file *file)
{
    if (!mutex_trylock(&fib_mutex)) {
        printk(KERN_ALERT "fibdrv is in use");
        return -EBUSY;
    }
    return 0;
}

static int fib_release(struct inode *inode, struct file *file)
{
    mutex_unlock(&fib_mutex);
    return 0;
}

/* calculate the fibonacci number at given offset */
static ssize_t fib_read(struct file *file,
                        char *buf,
                        size_t size,
                        loff_t *offset)
{
    ktime_t ktime = ktime_get();
#ifdef FAST_FIBONACCI
    if (*offset == MAX_LENGTH + 1)
        return ktime_to_ns(multi_ktime);
    struct BigN ret = {0, 0};
    fib_sequence(*offset, &ret);
    copy_to_user(buf, &ret, size);
#else
    fib_sequence(*offset, buf, size);
#endif
    unsigned int ns = ktime_to_ns(ktime_sub(ktime_get(), ktime));
    return ns;
}

/* write operation is skipped */
static ssize_t fib_write(struct file *file,
                         const char *buf,
                         size_t size,
                         loff_t *offset)
{
    return 1;
}

static loff_t fib_device_lseek(struct file *file, loff_t offset, int orig)
{
    loff_t new_pos = 0;
    switch (orig) {
    case 0: /* SEEK_SET: */
        new_pos = offset;
        break;
    case 1: /* SEEK_CUR: */
        new_pos = file->f_pos + offset;
        break;
    case 2: /* SEEK_END: */
        new_pos = MAX_LENGTH - offset;
        break;
    }

    if (new_pos > MAX_LENGTH + 1)
        new_pos = MAX_LENGTH;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    return new_pos;
}

const struct file_operations fib_fops = {
    .owner = THIS_MODULE,
    .read = fib_read,
    .write = fib_write,
    .open = fib_open,
    .release = fib_release,
    .llseek = fib_device_lseek,
};

static int __init init_fib_dev(void)
{
    int rc = 0;

    mutex_init(&fib_mutex);

    // Let's register the device
    // This will dynamically allocate the major number
    rc = alloc_chrdev_region(&fib_dev, 0, 1, DEV_FIBONACCI_NAME);

    if (rc < 0) {
        printk(KERN_ALERT
               "Failed to register the fibonacci char device. rc = %i",
               rc);
        return rc;
    }

    fib_cdev = cdev_alloc();
    if (fib_cdev == NULL) {
        printk(KERN_ALERT "Failed to alloc cdev");
        rc = -1;
        goto failed_cdev;
    }
    cdev_init(fib_cdev, &fib_fops);
    rc = cdev_add(fib_cdev, fib_dev, 1);

    if (rc < 0) {
        printk(KERN_ALERT "Failed to add cdev");
        rc = -2;
        goto failed_cdev;
    }

    fib_class = class_create(THIS_MODULE, DEV_FIBONACCI_NAME);

    if (!fib_class) {
        printk(KERN_ALERT "Failed to create device class");
        rc = -3;
        goto failed_class_create;
    }

    if (!device_create(fib_class, NULL, fib_dev, NULL, DEV_FIBONACCI_NAME)) {
        printk(KERN_ALERT "Failed to create device");
        rc = -4;
        goto failed_device_create;
    }
    return rc;
failed_device_create:
    class_destroy(fib_class);
failed_class_create:
    cdev_del(fib_cdev);
failed_cdev:
    unregister_chrdev_region(fib_dev, 1);
    return rc;
}

static void __exit exit_fib_dev(void)
{
    mutex_destroy(&fib_mutex);
    device_destroy(fib_class, fib_dev);
    class_destroy(fib_class);
    cdev_del(fib_cdev);
    unregister_chrdev_region(fib_dev, 1);
}

module_init(init_fib_dev);
module_exit(exit_fib_dev);
