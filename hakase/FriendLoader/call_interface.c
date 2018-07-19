#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sysfs.h>

#include "call_interface.h"
#include "common.h"
#include "common/_memory.h"

static struct kobject *call_sysfs_kobj;

static int call_h2f_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma);
static int call_f2h_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma);

static int call_i2h_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma);

static struct bin_attribute call_h2f_attr = {
    .attr =
        {
            .name = "h2f", .mode = S_IWUSR | S_IRUGO,
        },
    .size = 4096,
    .mmap = call_h2f_mmap,
};

static struct bin_attribute call_f2h_attr = {
    .attr =
        {
            .name = "f2h", .mode = S_IWUSR | S_IRUGO,
        },
    .size = 4096,
    .mmap = call_f2h_mmap,
};

static struct bin_attribute call_i2h_attr = {
    .attr =
        {
            .name = "i2h", .mode = S_IWUSR | S_IRUGO,
        },
    .size = 4096,
    .mmap = call_i2h_mmap,
};

static struct bin_attribute *call_sysfs_attrs[] = {
    &call_h2f_attr, &call_f2h_attr, &call_i2h_attr, NULL,
};
static struct attribute_group call_sysfs_attr_group = {
    .bin_attrs = call_sysfs_attrs,
};

void mmap_open(struct vm_area_struct *vma) {}

static int mmap_fault(struct vm_fault *vmf) {
  pr_err("frined_loader:mmap_fault\n");
  return -ENOSYS;
}

struct vm_operations_struct mmap_vm_ops = {
    .open = mmap_open, .fault = mmap_fault,
};

int __init call_interface_init(void) {
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

void __exit call_interface_exit(void) { kobject_put(call_sysfs_kobj); }

static int call_h2f_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma) {
  if (vma->vm_pgoff > 0) {
    return -EINVAL;
  }

  vma->vm_ops = &mmap_vm_ops;

  if (remap_pfn_range(vma, vma->vm_start,
                      (DEPLOY_PHYS_ADDR_START + kMemoryMapH2f) >> PAGE_SHIFT,
                      vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
    return -EAGAIN;
  }

  return 0;
}

static int call_f2h_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma) {
  if (vma->vm_pgoff > 0) {
    return -EINVAL;
  }

  vma->vm_ops = &mmap_vm_ops;

  if (remap_pfn_range(vma, vma->vm_start,
                      (DEPLOY_PHYS_ADDR_START + kMemoryMapF2h) >> PAGE_SHIFT,
                      vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
    return -EAGAIN;
  }

  return 0;
}

static int call_i2h_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma) {
  if (vma->vm_pgoff > 0) {
    return -EINVAL;
  }

  vma->vm_ops = &mmap_vm_ops;

  if (remap_pfn_range(vma, vma->vm_start,
                      (DEPLOY_PHYS_ADDR_START + kMemoryMapI2h) >> PAGE_SHIFT,
                      vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
    return -EAGAIN;
  }

  return 0;
}
