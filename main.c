#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#include "common.h"
#include "cpu_hotplug.h"
#include "deploy_data.h"

MODULE_DESCRIPTION("Deploy file to physical memory");
MODULE_LICENSE("GPL v2");

// File to be deployed is passed to kernel through Sysfs.
static struct kobject* sysfs_kobj;

static ssize_t show_file(
    struct kobject* kobj,
    struct kobj_attribute* attr,
    char* buf);

// Stored file will be deployed to memory.
static ssize_t store_file(
    struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf,
    size_t count);

static struct kobj_attribute file_attr
    = __ATTR(file, 0664, show_file, store_file);

static struct attribute* attrs[] = {
    &file_attr.attr,
    NULL,
};
static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int __init depftom_init(void)
{
    int ret;

    pr_info("depftom_init: init");

    // Sysfs init
    sysfs_kobj = kobject_create_and_add("file", &THIS_MODULE->mkobj.kobj);
    if (!sysfs_kobj) {
        pr_warn("depftom_init: kobject_create_and_add failed");
        return -1;
    }

    if (sysfs_create_group(sysfs_kobj, &attr_group)) {
        kobject_put(sysfs_kobj);
    }

    // Unplug CPU
    ret = cpu_unplug();
    if (ret < 0) {
        pr_warn("depftom_init: cpu_unplug failed: %d\n", ret);
        return -1;
    } else {
        pr_info("depftom_init: cpu %d down", ret);
    }

    return 0;
}

static void __exit depftom_exit(void)
{
    int ret = cpu_replug();
    if (ret < 0) {
        pr_warn("depftom_exit: cpu_replug failed: %d\n", ret);
    } else {
        pr_info("depftom_exit: cpu %d up", ret);
    }

    kobject_put(sysfs_kobj);

    pr_info("depftom_exit: exit");
}

static ssize_t show_file(
    struct kobject* kobj,
    struct kobj_attribute* attr,
    char* buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", "file to be deployed");
}

static ssize_t store_file(
    struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf,
    size_t count)
{
    struct deployment_info depinfo;

    if (deploy_data(buf, count, &depinfo) == -1) {
        pr_warn("depftom_init: deploy_data failed");
        return -1;
    } else {
        pr_info("depftom_init: "
                "deployed to physical memory [%llx, %llx) (offset = %llx)",
            depinfo.phys_addr,
            depinfo.phys_addr + count,
            depinfo.phys_offset);
    }

    return count;
}

module_init(depftom_init);
module_exit(depftom_exit);
