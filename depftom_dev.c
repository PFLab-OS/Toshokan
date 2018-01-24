#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "depftom_dev.h"
#include "deploy_data.h"

#define DEV_NAME "depftom"
#define MINOR_COUNT 1

static dev_t depftom_dev_id;
static struct cdev depftom_chardev;

static int depftom_dev_open(struct inode* inode, struct file* filep);
static int depftom_dev_release(struct inode* inode, struct file* filep);
static ssize_t depftom_dev_write(
    struct file* filep, const char __user* buf, size_t count, loff_t* offset);

static struct file_operations depftom_fops = {
    .owner = THIS_MODULE,
    .open = depftom_dev_open,
    .release = depftom_dev_release,
    .write = depftom_dev_write,
    .llseek = generic_file_llseek,
};

int __init depftom_dev_init(void)
{
    int ret = alloc_chrdev_region(&depftom_dev_id, 0, MINOR_COUNT, DEV_NAME);
    if (ret < 0) {
        pr_warn("depftom_dev_init: alloc device number failed: %d\n", ret);
        return -1;
    }

    cdev_init(&depftom_chardev, &depftom_fops);
    depftom_chardev.owner = THIS_MODULE;
    if (cdev_add(&depftom_chardev, depftom_dev_id, MINOR_COUNT)) {
        pr_warn("depftom_dev_init: failed to add cdev\n");
        return -1;
    }

    pr_info(
        "depftom_init: please run 'mknod /dev/depftom c %d 0'\n",
        MAJOR(depftom_dev_id));

    return 0;
}

void __exit depftom_dev_exit(void)
{
    cdev_del(&depftom_chardev);
    unregister_chrdev_region(depftom_dev_id, MINOR_COUNT);
}

static int depftom_dev_open(struct inode* inode, struct file* filep)
{
    pr_debug("depftom_dev: open\n");
    return 0;
}

static int depftom_dev_release(struct inode* inode, struct file* filep)
{
    pr_debug("depftom_dev: release\n");
    return 0;
}

static ssize_t depftom_dev_write(
    struct file* filep, const char __user* buf, size_t count, loff_t* offset)
{
    char* tmp_buf = (char*)kmalloc(count, GFP_KERNEL);
    if (tmp_buf == NULL) {
        return -ENOSPC;
    }

    if (copy_from_user(tmp_buf, buf, count)) {
        kfree(tmp_buf);
        return -EFAULT;
    }

    if (deploy_data(tmp_buf, count, filep->f_pos) < 0) {
        pr_warn("depftom: deploy_data failed\n");
        kfree(tmp_buf);
        return -EFBIG;
    }

    *offset += count;
    kfree(tmp_buf);

    return count;
}
