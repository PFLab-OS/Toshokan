#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/realmode.h>
#include <asm/uv/uv.h>

#include "common.h"
#include "cpu_hotplug.h"
#include "deploy_data.h"

MODULE_DESCRIPTION("Deploy file to physical memory");
MODULE_LICENSE("GPL v2");

#define DEVNAME "depftom"

static int depftom_dev_major;
static struct cdev cdev_st;
static struct file_operations depftom_fops;

static dma_addr_t paddr;
static const size_t buf_size = 4096;

static int start_cpu(int unpluged_cpu) {
  int apicid;
  int boot_error;
    
  apicid = apic->cpu_present_to_apicid(unpluged_cpu);

  if (get_uv_system_type() != UV_NON_UNIQUE_APIC) {
    smpboot_setup_warm_reset_vector(paddr);
  }
  
  preempt_disable();

  /*
   * Wake up AP by INIT, INIT, STARTUP sequence.
   */
  boot_error = wakeup_secondary_cpu_via_init(apicid, paddr);

  preempt_enable();

  return boot_error;
}

static int __init depftom_init(void)
{
  int ret;
  dev_t depftom_dev;
  uint32_t __iomem* io_addr;
  uint8_t buf[] = {0xFA, 0xF4, 0xF4, 0xFA};
  static const size_t bin_size = sizeof(buf) / sizeof(buf[0]);

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
  if(cdev_add(&cdev_st, MKDEV(depftom_dev_major, 0), 1)) {
    pr_warn("depftom_init: fail to add cdev\n");
    return -1;
  }

  pr_info("depftom_init: please run 'mknod /dev/depftom c %d 0'\n", depftom_dev_major);

  for (paddr = 0x1000; paddr < 0x100000; paddr += buf_size) {
    int i = 0;
    int flag = 0;
    io_addr = ioremap(paddr, buf_size);
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
      	flag = 1;
      }
    }
    iounmap(io_addr);
    if (flag == 1) {
      break;
    }
  }

  if (paddr == 0x100000) {
    pr_warn("depftom_init: no trampoline space\n");
    return -1;
  }

  io_addr = ioremap(paddr, buf_size);

  memcpy_toio(io_addr, buf, bin_size);
    
  iounmap(io_addr);

  // Unplug CPU
  ret = cpu_unplug();
  if (ret < 0) {
    pr_warn("depftom_init: cpu_unplug failed: %d\n", ret);
    return -1;
  } else {
    pr_info("depftom_init: cpu %d down\n", ret);
  }

  pr_info("depftom_init: start cpu from %llx\n", paddr);
  ret = start_cpu(ret);

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
