#include <linux/kernel.h>
#include <linux/sysfs.h>

#include "call_interface.h"

static struct kobject* call_sysfs_kobj;

static int call_h2f_mmap(struct file *filep, struct kobject *kobj, struct bin_attribute *attr,
		    struct vm_area_struct *vma);
static int call_f2h_mmap(struct file *filep, struct kobject *kobj, struct bin_attribute *attr,
		    struct vm_area_struct *vma);

static struct bin_attribute call_h2f_attr = {
	.attr = {
    .name = "h2f",
    .mode = S_IWUSR,
  },
  .size = 4 * 1024, // 4KB
  .mmap = call_h2f_mmap,
};

static struct bin_attribute call_f2h_attr = {
	.attr = {
    .name = "f2h",
    .mode = S_IWUSR,
  },
  .size = 4 * 1024, // 4KB
  .mmap = call_f2h_mmap,
};

static struct bin_attribute* call_sysfs_attrs[] = {
    &call_h2f_attr,
    &call_f2h_attr,
    NULL,
};
static struct attribute_group call_sysfs_attr_group = {
    .bin_attrs = call_sysfs_attrs,
};

int __init call_interface_init(void)
{
  int ret;
  
  /* sysfs for call binary */
  call_sysfs_kobj = kobject_create_and_add("call", &THIS_MODULE->mkobj.kobj);
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

void __exit call_interface_exit(void)
{
    kobject_put(call_sysfs_kobj);
}

static int call_h2f_mmap(struct file *filep, struct kobject *kobj, struct bin_attribute *attr,
		    struct vm_area_struct *vma)
{
  return 0;
}

static int call_f2h_mmap(struct file *filep, struct kobject *kobj, struct bin_attribute *attr,
		    struct vm_area_struct *vma)
{
  return 0;
}
