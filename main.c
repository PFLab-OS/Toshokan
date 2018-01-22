#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#include "common.h"
#include "cpu_hotplug.h"
#include "deploy_data.h"

MODULE_DESCRIPTION("Deploy file to physical memory");
MODULE_LICENSE("GPL v2");

#define DEVNAME "depftom"

static int depftom_dev_major;
static struct cdev cdev_st;
static struct file_operations depftom_fops;

static const size_t buf_size = 4096;

struct trampoline_region {
  dma_addr_t paddr;
  uint32_t* vaddr;
};
static struct trampoline_region region;

static int start_cpu(int unpluged_cpu, struct trampoline_region *region);
static int alloc_trampoline_region(struct trampoline_region *region);
static void free_trampoline_region(struct trampoline_region *region);

#ifdef CONFIG_X86_64
#include <asm/realmode.h>
#include <asm/uv/uv.h>

static int start_cpu(int unpluged_cpu, struct trampoline_region *region) {
  int apicid;
  int boot_error;
    
  apicid = apic->cpu_present_to_apicid(unpluged_cpu);

  if (get_uv_system_type() != UV_NON_UNIQUE_APIC) {
    smpboot_setup_warm_reset_vector(region->paddr);
  }
  
  preempt_disable();

  /*
   * Wake up AP by INIT, INIT, STARTUP sequence.
   */
  boot_error = wakeup_secondary_cpu_via_init(apicid, region->paddr);

  preempt_enable();

  return boot_error;
}

static int alloc_trampoline_region(struct trampoline_region *region) {
  dma_addr_t tpaddr;
  for (tpaddr = 0x1000; tpaddr < 0x100000; tpaddr += buf_size) {
    int i = 0;
    int flag = 0;
    uint32_t* io_addr = ioremap(tpaddr, buf_size);
    if (io_addr == 0) {
      continue;
    }
    if (io_addr[0] == FRIEND_LOADER_TRAMPOLINE_SIGNATURE) {
      for (i = 1; i < (buf_size / sizeof(uint32_t)); i++) {
      	if (io_addr[i] != 0) {
      	  break;
      	}
      }
      if (i == buf_size / sizeof(uint32_t)) {
	region->paddr = tpaddr;
	region->vaddr = io_addr;
	return 0;
      }
    }
    iounmap(io_addr);
  }
  return -1;
}

static void free_trampoline_region(struct trampoline_region *region) {
  iounmap(region->vaddr);
}

static uint8_t bin[] = {0xFA, 0xF4, 0xF4, 0xFA};

#endif /* CONFIG_X86_64 */

#ifdef CONFIG_ARM64
#include <linux/psci.h>
#include <asm/smp_plat.h>
#include <asm/smp.h>

static int start_cpu(int unpluged_cpu, struct trampoline_region *region) {
  psci_ops.cpu_on(cpu_logical_map(unpluged_cpu), (unsigned long)region->paddr);
  return 0;
}

static int alloc_trampoline_region(struct trampoline_region *region) {
  if (__friend_loader_buf[0] != FRIEND_LOADER_TRAMPOLINE_SIGNATURE) {
    return -1;
  }

  region->paddr = __pa_symbol(__friend_loader_buf);
  region->vaddr = __friend_loader_buf;
  
  return 0;
}

static void free_trampoline_region(struct trampoline_region *region) {
}

static uint8_t bin[] = {
  0x00, 0x00, 0x84, 0xd2, // mov x0, 0x2000
  0x60, 0xfe, 0xbf, 0xf2, // movk x0, 0xfff3, lsl 16
  0x21, 0x08, 0x80, 0x52, // mov w1, 65
  0x01, 0x00, 0x00, 0x39, // strb w1, [x0]
  // loop:
  0x5f, 0x20, 0x03, 0xd5, // wfe
  0xff, 0xff, 0xff, 0x17, // b loop;
};

#endif /* CONFIG_ARM64 */

static const size_t bin_size = sizeof(bin) / sizeof(bin[0]);

static int __init depftom_init(void)
{
  int ret;
  dev_t depftom_dev;

  pr_info("depftom_init: init\n");

  // alloc decive number
  ret = alloc_chrdev_region(&depftom_dev, 0, 1, DEVNAME);
  
  if (ret < 0) {
    pr_warn("depftom_init: alloc device number failed: %d\n", ret);
    return -1;
  }
  
  cdev_init(&cdev_st, &depftom_fops);
  cdev_st.owner = THIS_MODULE;
  depftom_dev_major = MAJOR(depftom_dev);
  if (cdev_add(&cdev_st, MKDEV(depftom_dev_major, 0), 1)) {
    pr_warn("depftom_init: fail to add cdev\n");
    return -1;
  }

  pr_info("depftom_init: please run 'mknod /dev/depftom c %d 0'\n", depftom_dev_major);

  if (alloc_trampoline_region(&region) < 0) {
    pr_warn("depftom_init: no trampoline space\n");
    return -1;
  }

  memcpy(region.vaddr, bin, bin_size);
    
  // Unplug CPU
  ret = cpu_unplug();
  if (ret < 0) {
    pr_warn("depftom_init: cpu_unplug failed: %d\n", ret);
    return -1;
  } else {
    pr_info("depftom_init: cpu %d down\n", ret);
  }

  // TODO : sysctlで1を書き込まれたら起動する、みたいな形に変更する事
  pr_info("depftom_init: start cpu from %llx\n", region.paddr);
  ret = start_cpu(ret, &region);

  return ret;
}

static void __exit depftom_exit(void)
{
  int ret = cpu_replug();
  if (ret < 0) {
    pr_warn("depftom_exit: cpu_replug failed: %d\n", ret);
  } else {
    pr_info("depftom_exit: cpu %d up\n", ret);
  }

  cdev_del(&cdev_st);
  unregister_chrdev_region(MKDEV(depftom_dev_major, 0), 1);

  free_trampoline_region(&region);

  pr_info("depftom_exit: exit\n");
}

static int depftom_dev_open(struct inode *inode, struct file *filep)
{
  pr_info("depftom_init: open\n");
  return 0;
}

static int depftom_dev_release(struct inode *inode, struct file *filep)
{
  return 0;
}

static ssize_t depftom_dev_write(struct file *filep, const char __user *buf,
				 size_t count, loff_t *offset)
{
  char *tmp_buf = (char*)kmalloc(count, GFP_KERNEL);
  if (tmp_buf == NULL) {
    return -ENOSPC;
  }
  
  if(copy_from_user(tmp_buf, buf, count)) {
    kfree(tmp_buf);
    return -EFAULT;
  }

  if (deploy_data(tmp_buf, count, filep->f_pos) == -1) {
    pr_warn("depftom_init: deploy_data failed\n");
    kfree(tmp_buf);
    return -EFBIG;
  }

  *offset += count;
  
  kfree(tmp_buf);
  return count;
}

static struct file_operations depftom_fops = {
  .open      = depftom_dev_open,
  .release   = depftom_dev_release,
  .write     = depftom_dev_write,
};

module_init(depftom_init);
module_exit(depftom_exit);
