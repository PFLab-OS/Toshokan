#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "dumper_dev.h"

#define DEV_NAME "friend_dumper"
#define MINOR_COUNT 1

static dev_t dumper_dev_id;
static struct cdev dumper_chardev;

static int dumper_dev_open(struct inode* inode, struct file* filep);
static int dumper_dev_release(struct inode* inode, struct file* filep);
static ssize_t dumper_dev_write(
    struct file* filep, const char __user* buf, size_t count, loff_t* offset);
static ssize_t dumper_dev_read(
    struct file* filep, char __user* buf, size_t count, loff_t* offset);

static struct file_operations dumper_fops = {
    .owner = THIS_MODULE,
    .open = dumper_dev_open,
    .release = dumper_dev_release,
    .write = dumper_dev_write,
    .read = dumper_dev_read,
    .llseek = generic_file_llseek,
};

static char* read_buf;

int __init dumper_dev_init(void)
{
    int ret = alloc_chrdev_region(&dumper_dev_id, 0, MINOR_COUNT, DEV_NAME);
    if (ret < 0) {
        pr_warn("dumper_dev_init: alloc device number failed: %d\n", ret);
        return -1;
    }

    cdev_init(&dumper_chardev, &dumper_fops);
    dumper_chardev.owner = THIS_MODULE;
    if (cdev_add(&dumper_chardev, dumper_dev_id, MINOR_COUNT)) {
        pr_warn("dumper_dev_init: failed to add cdev\n");
        return -1;
    }

    pr_info(
        "dumper_init: please run 'mknod /dev/friend_dumper c %d 0'\n",
        MAJOR(dumper_dev_id));

    read_buf = (char*)kmalloc(KMALLOC_MAX_SIZE, GFP_KERNEL);
    if (read_buf == NULL) {
        pr_warn("dumper_dev_init: failed to allocate buf\n");
        return -ENOSPC;
    }

    return 0;
}

void __exit dumper_dev_exit(void)
{
    kfree(read_buf);

    cdev_del(&dumper_chardev);
    unregister_chrdev_region(dumper_dev_id, MINOR_COUNT);
}

static int dumper_dev_open(struct inode* inode, struct file* filep)
{
    return 0;
}

static int dumper_dev_release(struct inode* inode, struct file* filep)
{
    return 0;
}

static ssize_t dumper_dev_write(
    struct file* filep, const char __user* buf, size_t count, loff_t* offset)
{
    return count;
}

static void read_phys_mem(void* buf, phys_addr_t addr, size_t size);

static ssize_t dumper_dev_read(
    struct file* filep, char __user* buf, size_t count, loff_t* offset)
{
    size_t read_size = 0;
    size_t size;

    while (read_size < count) {
        size = min(count - read_size, KMALLOC_MAX_SIZE);

        pr_info("friend_dumper: %d\n", __LINE__);
        read_phys_mem(
            read_buf, __pa_symbol(__friend_loader_buf) + filep->f_pos, size);
        pr_info("friend_dumper: %d\n", __LINE__);

        if (copy_to_user(buf + read_size, read_buf, size)) {
            pr_warn("friend_dumper: copy_to_user failed\n");
            return -EFAULT;
        }
        pr_info("friend_dumper: %d\n", __LINE__);

        read_size += size;
    }

    *offset += count;

    return count;
}

static void read_phys_mem(void* buf, phys_addr_t addr, size_t size)
{
    void __iomem* io_addr = ioremap(addr, size);
    pr_info("friend_dumper: %d\n", __LINE__);
    memcpy_fromio(buf, io_addr, size);
    pr_info("friend_dumper: %d\n", __LINE__);
    iounmap(io_addr);
    pr_info("friend_dumper: %d\n", __LINE__);
}
