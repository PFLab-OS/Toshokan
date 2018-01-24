#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysfs.h>

#include "common.h"
#include "cpu_hotplug.h"
#include "depftom_dev.h"

MODULE_DESCRIPTION("Friend Loader");
MODULE_LICENSE("GPL v2");

// Sysfs for start booting friend core
static struct kobject* boot_sysfs_kobj;

static ssize_t boot_sysfs_read(
    struct kobject* kobj,
    struct kobj_attribute* attr,
    char* buf);

static ssize_t boot_sysfs_write(
    struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf,
    size_t count);

static struct kobj_attribute boot_sysfs_attr
    = __ATTR(boot, 0664, boot_sysfs_read, boot_sysfs_write);

static struct attribute* boot_sysfs_attrs[] = {
    &boot_sysfs_attr.attr,
    NULL,
};
static struct attribute_group boot_sysfs_attr_group = {
    .attrs = boot_sysfs_attrs,
};

static int __init depftom_init(void)
{
    int ret;

    pr_info("depftom_init: init\n");

    // Device for storing program
    depftom_dev_init();

    // Sysfs for start booting friend core
    boot_sysfs_kobj = kobject_create_and_add("boot", &THIS_MODULE->mkobj.kobj);
    if (!boot_sysfs_kobj) {
        pr_warn("depftom_init: kobject_create_and_add failed");
        return -1;
    }

    ret = sysfs_create_group(boot_sysfs_kobj, &boot_sysfs_attr_group);
    if (ret != 0) {
        kobject_put(boot_sysfs_kobj);
        pr_warn("depftom_init: sysfs_create_group failed: %d\n", ret);
        return -1;
    }

    // Unplug friend core
    ret = cpu_unplug();
    if (ret < 0) {
        pr_warn("depftom_init: cpu_unplug failed: %d\n", ret);
        return -1;
    }

    pr_info("depftom_init: cpu %d down\n", ret);

    return 0;
}

static void __exit depftom_exit(void)
{
    int ret = cpu_replug();
    if (ret < 0) {
        pr_warn("depftom_exit: cpu_replug failed: %d\n", ret);
    } else {
        pr_info("depftom_exit: cpu %d up\n", ret);
    }

    kobject_put(boot_sysfs_kobj);
    depftom_dev_exit();

    pr_info("depftom_exit: exit\n");
}

static ssize_t boot_sysfs_read(
    struct kobject* kobj,
    struct kobj_attribute* attr,
    char* buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", "boot_sysfs: read");
}

static ssize_t boot_sysfs_write(
    struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf,
    size_t count)
{
    if (cpu_start(DEPLOY_PHYS_ADDR_START) == 0) {
        pr_info("depftom: starting cpu from 0x%lx\n", DEPLOY_PHYS_ADDR_START);
    } else {
        pr_warn("depftom: cpu_start failed\n");
    }

    return (ssize_t)count;
}

module_init(depftom_init);
module_exit(depftom_exit);
