#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "deploy_dev.h"
#include "deploy.h"

#define DEV_NAME "friend_loader"
#define MINOR_COUNT 1

static dev_t deploy_dev_id;
static struct cdev deploy_chardev;

static int deploy_dev_open(struct inode* inode, struct file* filep);
static int deploy_dev_release(struct inode* inode, struct file* filep);
static ssize_t deploy_dev_write(
    struct file* filep, const char __user* buf, size_t count, loff_t* offset);

static struct file_operations deploy_fops = {
    .owner = THIS_MODULE,
    .open = deploy_dev_open,
    .release = deploy_dev_release,
    .write = deploy_dev_write,
    .llseek = generic_file_llseek,
};

static char* write_buf;

int __init deploy_dev_init(void)
{
    int ret = alloc_chrdev_region(&deploy_dev_id, 0, MINOR_COUNT, DEV_NAME);
    if (ret < 0) {
        pr_warn("deploy_dev_init: alloc device number failed: %d\n", ret);
        return -1;
    }

    cdev_init(&deploy_chardev, &deploy_fops);
    deploy_chardev.owner = THIS_MODULE;
    if (cdev_add(&deploy_chardev, deploy_dev_id, MINOR_COUNT)) {
        pr_warn("deploy_dev_init: failed to add cdev\n");
        return -1;
    }

    pr_info(
        "deploy_init: please run 'mknod /dev/friend_loader c %d 0'\n",
        MAJOR(deploy_dev_id));

    write_buf = (char*)kmalloc(KMALLOC_MAX_SIZE, GFP_KERNEL);
    if (write_buf == NULL) {
        pr_warn("deploy_dev_init: failed to allocate buf\n");
        return -ENOSPC;
    }

    return 0;
}

void __exit deploy_dev_exit(void)
{
    kfree(write_buf);

    cdev_del(&deploy_chardev);
    unregister_chrdev_region(deploy_dev_id, MINOR_COUNT);
}

static int deploy_dev_open(struct inode* inode, struct file* filep)
{
    pr_debug("deploy_dev: open\n");
    return 0;
}

static int deploy_dev_release(struct inode* inode, struct file* filep)
{
    pr_debug("deploy_dev: release\n");
    return 0;
}

static ssize_t deploy_dev_write(
    struct file* filep, const char __user* buf, size_t count, loff_t* offset)
{
    size_t written_size = 0;
    size_t size;

    while (written_size < count) {
        size = min(count - written_size, KMALLOC_MAX_SIZE);

        if (copy_from_user(write_buf, buf + written_size, size)) {
            pr_warn("friend_loader: copy_from_user failed\n");
            return -EFAULT;
        }

        if (deploy(write_buf, size, filep->f_pos + written_size) < 0) {
            pr_warn("friend_loader: deploy failed\n");
            return -EFBIG;
        }

        written_size += size;
    }

    *offset += count;

    return count;
}
