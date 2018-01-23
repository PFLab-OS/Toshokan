#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "depftom_dev.h"
#include "deploy_data.h"

int depftom_dev_major;

static int depftom_dev_open(struct inode* inode, struct file* filep);
static int depftom_dev_release(struct inode* inode, struct file* filep);
static ssize_t depftom_dev_write(
    struct file* filep, const char __user* buf, size_t count, loff_t* offset);

static struct file_operations depftom_fops = {
    .open = depftom_dev_open,
    .release = depftom_dev_release,
    .write = depftom_dev_write,
};

#define DEV_NAME "depftom"

static struct cdev cdev_st;

int devftom_dev_init(void)
{
    dev_t depftom_dev;

    int ret = alloc_chrdev_region(&depftom_dev, 0, 1, DEV_NAME);
    if (ret < 0) {
        pr_warn("depftom_dev_init: alloc device number failed: %d\n", ret);
        return -1;
    }

    cdev_init(&cdev_st, &depftom_fops);
    cdev_st.owner = THIS_MODULE;
    depftom_dev_major = MAJOR(depftom_dev);
    if (cdev_add(&cdev_st, MKDEV(depftom_dev_major, 0), 1)) {
        pr_warn("depftom_dev_init: failed to add cdev\n");
        return -1;
    }

    return 0;
}

void depftom_dev_exit(void)
{
    cdev_del(&cdev_st);
    unregister_chrdev_region(MKDEV(depftom_dev_major, 0), 1);
}

static int depftom_dev_open(struct inode* inode, struct file* filep)
{
    (void)inode;
    (void)filep;

    pr_info("depftom_dev: open\n");

    return 0;
}

static int depftom_dev_release(struct inode* inode, struct file* filep)
{
    (void)inode;
    (void)filep;

    pr_info("depftom_dev: release\n");

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
