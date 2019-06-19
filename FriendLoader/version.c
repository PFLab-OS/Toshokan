#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sysfs.h>

#include "version.h"
#include <toshokan/version.h>

static struct kobject *call_sysfs_kobj;

static ssize_t call_read(struct file *fp, struct kobject *kobj, struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count) {
  memcpy(buf, version_string + off, count);
  return count;
}

static struct bin_attribute call_version_attr = {
    .attr =
        {
            .name = "version", .mode = S_IRUGO,
        },
    .size = sizeof(version_string),
    .read = call_read,
};

static struct bin_attribute *call_sysfs_attrs[] = {
  &call_version_attr, NULL,
};

static struct attribute_group call_sysfs_attr_group = {
    .bin_attrs = call_sysfs_attrs,
};


int __init version_interface_init(void) {
  int ret;

  call_sysfs_kobj = kobject_create_and_add("info", &THIS_MODULE->mkobj.kobj);
  if (!call_sysfs_kobj) {
    pr_warn("friend_loader_init: kobject_create_and_add failed");
    return -1;
  }

  ret = sysfs_create_group(call_sysfs_kobj, &call_sysfs_attr_group);
  if (ret != 0) {
    kobject_put(call_sysfs_kobj);
    pr_warn("friend_loader_init: sysfs_create_group failed: %d\n", ret);
    return -1;
  }

  return 0;
}

void __exit version_interface_exit(void) { kobject_put(call_sysfs_kobj); }
