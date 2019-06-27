#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/cdev.h>

#include <toshokan/memory.h>
#include "call_interface.h"

#define DRIVER_NAME "FriendLoader"

static struct cdev memdevice_cdev;
static dev_t memdevice_dev;

static struct cdev bootmemdevice_cdev;
static dev_t bootmemdevice_dev;

static void mmap_open(struct vm_area_struct *vma) {}

static int mmap_fault(struct vm_fault *vmf) {
  pr_err("frined_loader:mmap_fault\n");
  return -ENOSYS;
}

struct vm_operations_struct mmap_vm_ops = {
    .open = mmap_open, .fault = mmap_fault,
};

static int memdevice_open(struct inode *inode, struct file *file)
{
  return 0;
}

static int memdevice_close(struct inode *inode, struct file *file)
{
  return 0;
}

static int memdevice_mmap(struct file *file, struct vm_area_struct *vma) {
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

static int bootmemdevice_mmap(struct file *file, struct vm_area_struct *vma) {
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

struct file_operations s_memdevice_fops = {
  .open    = memdevice_open,
  .mmap    = memdevice_mmap,
  .release = memdevice_close,
};

struct file_operations s_bootmemdevice_fops = {
  .open    = memdevice_open,
  .mmap    = bootmemdevice_mmap,
  .release = memdevice_close,
};

static struct class *memdevice_class = NULL;
static struct class *bootmemdevice_class = NULL;

int __init memdevice_init(void) {
  // for mem
  if (alloc_chrdev_region(&memdevice_dev, 0, 1, DRIVER_NAME) != 0) {
    pr_err("friend_loader_init: failed to alloc_chrdev_regionn");
    return -ENXIO;
  }
  memdevice_class = class_create(THIS_MODULE, "friend_mem");
  if (IS_ERR(memdevice_class)) {
    pr_err("friend_loader_init: failed to class_create\n");
    unregister_chrdev_region(memdevice_dev, 1);
    return -ENXIO;
  }
  
  if (device_create(memdevice_class, NULL, memdevice_dev, NULL, "friend_mem") == NULL) {
    class_destroy(memdevice_class);
    unregister_chrdev_region(memdevice_dev, 1);
    return -ENXIO;
  }
  
  cdev_init(&memdevice_cdev, &s_memdevice_fops);
  if (cdev_add(&memdevice_cdev, memdevice_dev, 1) != 0) {
    device_destroy(memdevice_class, memdevice_dev);
    class_destroy(memdevice_class);
    unregister_chrdev_region(memdevice_dev, 1);
    return -ENXIO;
  }

  // for bootmem
  if (alloc_chrdev_region(&bootmemdevice_dev, 0, 1, DRIVER_NAME) != 0) {
    pr_err("friend_loader_init: failed to alloc_chrdev_regionn");
    return -ENXIO;
  }
  bootmemdevice_class = class_create(THIS_MODULE, "friend_bootmem");
  if (IS_ERR(bootmemdevice_class)) {
    pr_err("friend_loader_init: failed to class_create\n");
    unregister_chrdev_region(bootmemdevice_dev, 1);
    return -ENXIO;
  }
  
  if (device_create(bootmemdevice_class, NULL, bootmemdevice_dev, NULL, "friend_bootmem"TRAMPOLINE_ADDR_STR) == NULL) {
    class_destroy(bootmemdevice_class);
    unregister_chrdev_region(bootmemdevice_dev, 1);
    return -ENXIO;
  }
  
  cdev_init(&bootmemdevice_cdev, &s_bootmemdevice_fops);
  if (cdev_add(&bootmemdevice_cdev, bootmemdevice_dev, 1) != 0) {
    device_destroy(bootmemdevice_class, bootmemdevice_dev);
    class_destroy(bootmemdevice_class);
    unregister_chrdev_region(bootmemdevice_dev, 1);
    return -ENXIO;
  }

  return 0;
}

void __exit memdevice_exit(void) {
  // for mem
  cdev_del(&memdevice_cdev);
  device_destroy(memdevice_class, memdevice_dev);
  class_destroy(memdevice_class);
  unregister_chrdev_region(memdevice_dev, 1);

  // for bootmem
  cdev_del(&bootmemdevice_cdev);
  device_destroy(bootmemdevice_class, bootmemdevice_dev);
  class_destroy(bootmemdevice_class);
  unregister_chrdev_region(bootmemdevice_dev, 1);
}
