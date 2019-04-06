#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/cdev.h>

#include "cpudevice.h"
#include "cpu_hotplug.h"
#include "_memory.h"

#define DRIVER_NAME "FriendLoader"

static const unsigned int MINOR_BASE = 0;
static unsigned int minor_num = 0;

static unsigned int cpudevice_major;
static struct cdev cpudevice_cdev;

static int cpudevice_open(struct inode *inode, struct file *file)
{
  int cpuid = iminor(file->f_path.dentry->d_inode) - MINOR_BASE + 1;
  if (cpu_start(cpuid) == 0) {
    pr_info("friend_loader: starting processor core %d from 0x%lx\n",
	    cpuid, DEPLOY_PHYS_ADDR_START);
  } else {
    pr_info("friend_loader: failed starting processor core %d\n", cpuid);
  }
  return 0;
}

static int cpudevice_close(struct inode *inode, struct file *file)
{
  int cpuid = iminor(file->f_path.dentry->d_inode) - MINOR_BASE + 1;
  if (cpu_replug(cpuid) == 0) {
    pr_info("friend_loader: stopped processor core %d\n",
	    cpuid);
  } else {
    pr_info("friend_loader: failed to stop processor core %d\n", cpuid);
  }
  return 0;
}

struct file_operations s_cpudevice_fops = {
  .open    = cpudevice_open,
  .release = cpudevice_close,
};

static struct class *cpudevice_class = NULL;

int __init cpudevice_init(void) {
  dev_t dev;
  int minor;
  
  minor_num = num_possible_cpus() - 1;
  if (minor_num == 0) {
    /* no cpus for friends */
    return -EIO;
  }

  if (alloc_chrdev_region(&dev, MINOR_BASE, minor_num, DRIVER_NAME) != 0) {
    pr_err("friend_loader_init: failed to alloc_chrdev_regionn");
    return -ENXIO;
  }

  cpudevice_major = MAJOR(dev);
  dev = MKDEV(cpudevice_major, MINOR_BASE);

  cdev_init(&cpudevice_cdev, &s_cpudevice_fops);
  cpudevice_cdev.owner = THIS_MODULE;

  if (cdev_add(&cpudevice_cdev, dev, minor_num) != 0) {
    pr_err("friend_loader_init: failed to cdev_add\n");
    unregister_chrdev_region(dev, minor_num);
    return -ENXIO;
  }

  cpudevice_class = class_create(THIS_MODULE, "friend_cpu");
  if (IS_ERR(cpudevice_class)) {
    pr_err("friend_loader_init: failed to class_create\n");
    cdev_del(&cpudevice_cdev);
    unregister_chrdev_region(dev, minor_num);
    return -ENXIO;
  }

  for (minor = MINOR_BASE; minor < MINOR_BASE + minor_num; minor++) {
    device_create(cpudevice_class, NULL, MKDEV(cpudevice_major, minor), NULL, "friend_cpu%d", minor - MINOR_BASE + 1);
  }

  return 0;
}

void __exit cpudevice_exit(void) {
  int minor;
  dev_t dev = MKDEV(cpudevice_major, MINOR_BASE);
  
  for (minor = MINOR_BASE; minor < MINOR_BASE + minor_num; minor++) {
    device_destroy(cpudevice_class, MKDEV(cpudevice_major, minor));
  }

  class_destroy(cpudevice_class);
  cdev_del(&cpudevice_cdev);
  unregister_chrdev_region(dev, minor_num);
}
