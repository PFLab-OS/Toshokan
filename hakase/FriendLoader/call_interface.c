#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sysfs.h>

#include "call_interface.h"
#include "common.h"
#include "_memory.h"

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

static int call_mem_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma);

static int call_bootmem_mmap(struct file *filep, struct kobject *kobj,
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

static struct bin_attribute call_mem_attr = {
    .attr =
        {
            .name = "mem", .mode = S_IWUSR | S_IRUGO,
        },
    .size = DEPLOY_PHYS_MEM_SIZE,
    .mmap = call_mem_mmap,
};

static struct bin_attribute call_bootmem_attr = {
    .attr =
        {
            .name = "bootmem", .mode = S_IWUSR | S_IRUGO,
        },
    .size = PAGE_SIZE,
    .mmap = call_bootmem_mmap,
};

static struct bin_attribute *call_sysfs_attrs[] = {
  &call_h2f_attr, &call_f2h_attr, &call_i2h_attr, &call_mem_attr, &call_bootmem_attr, NULL,
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

// from drivers/char/mem.c
static int call_mem_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma) {
  size_t size = vma->vm_end - vma->vm_start;
  phys_addr_t phys_start = (phys_addr_t)vma->vm_pgoff << PAGE_SHIFT;
  phys_addr_t phys_end = phys_start + (phys_addr_t)size;
  
  /* It's illegal to wrap around the end of the physical address space. */
  if (phys_end - 1 < phys_start) {
    return -EINVAL;
  }

  if (phys_start > DEPLOY_PHYS_MEM_SIZE || phys_end > DEPLOY_PHYS_MEM_SIZE) {
    return -EINVAL;
  }

#ifdef ARCH_HAS_VALID_PHYS_ADDR_RANGE
  if (!valid_mmap_phys_addr_range(vma->vm_pgoff, size))
    return -EINVAL;
#endif
  
  vma->vm_ops = &mmap_vm_ops;

  /* Remap-pfn-range will mark the range VM_IO */
  if (remap_pfn_range(vma,
		      vma->vm_start,
		      vma->vm_pgoff + (DEPLOY_PHYS_ADDR_START >> PAGE_SHIFT),
		      size,
		      vma->vm_page_prot)) {
    return -EAGAIN;
  }
  return 0;
}

static int call_bootmem_mmap(struct file *filep, struct kobject *kobj,
                         struct bin_attribute *attr,
                         struct vm_area_struct *vma) {
  if (vma->vm_pgoff > 0) {
    return -EINVAL;
  }

  vma->vm_ops = &mmap_vm_ops;

  if (remap_pfn_range(vma, vma->vm_start,
                      TRAMPOLINE_ADDR >> PAGE_SHIFT,
                      vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
    return -EAGAIN;
  }

  return 0;
}

