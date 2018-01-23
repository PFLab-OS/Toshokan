#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysfs.h>

#include "cpu_hotplug.h"
#include "depftom_dev.h"
#include "deploy_data.h"

MODULE_DESCRIPTION("Deploy file to physical memory");
MODULE_LICENSE("GPL v2");

// Trampoline region
struct trampoline_region {
    phys_addr_t paddr;
    uint32_t* vaddr;
};
static struct trampoline_region region;

static int __init alloc_trampoline_region(struct trampoline_region* region);
static void __exit free_trampoline_region(struct trampoline_region* region);

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

static uint8_t bin[] = {
    0x00, 0x00, 0x84, 0xd2,  // mov x0, 0x2000
    0x60, 0xfe, 0xbf, 0xf2,  // movk x0, 0xfff3, lsl 16
    0x21, 0x08, 0x80, 0x52,  // mov w1, 65
    0x01, 0x00, 0x00, 0x39,  // strb w1, [x0]
    // loop:
    0x5f, 0x20, 0x03, 0xd5,  // wfe
    0xff, 0xff, 0xff, 0x17,  // b loop;
};

static const size_t bin_size = sizeof(bin) / sizeof(bin[0]);

static int __init depftom_init(void)
{
    int ret;

    pr_info("depftom_init: init\n");

    // Trampoline region
    if (alloc_trampoline_region(&region) < 0) {
        pr_warn("depftom_init: no trampoline space\n");
        return -1;
    }
    memcpy(region.vaddr, bin, bin_size);

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

    free_trampoline_region(&region);

    pr_info("depftom_exit: exit\n");
}

static int __init alloc_trampoline_region(struct trampoline_region* region)
{
    if (__friend_loader_buf[0] != FRIEND_LOADER_TRAMPOLINE_SIGNATURE) {
        pr_warn("alloc_trampoline_region: signature does not match\n");
        return -1;
    }

    region->paddr = __pa_symbol(__friend_loader_buf);
    region->vaddr = __friend_loader_buf;

    return 0;
}

static void __exit free_trampoline_region(struct trampoline_region* region) {}

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
    if (cpu_start(region.paddr) == 0) {
        pr_info("depftom: starting cpu from %llx\n", region.paddr);
    } else {
        pr_warn("depftom: failed to start cpu\n");
    }

    return (ssize_t)count;
}

module_init(depftom_init);
module_exit(depftom_exit);
