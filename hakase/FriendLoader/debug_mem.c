#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <linux/uaccess.h>
#include "common.h"

static struct dentry *topdir;
static struct dentry *memory;
static struct dentry *zero_clear;

//TODO using lock 
static unsigned int zero_clear_status = 0;

struct work_struct zero_clear_workq;

static ssize_t memory_read(struct file *file, char __user *buf, size_t len, loff_t *ppos) {
  void __iomem *io_addr;
  phys_addr_t addr = DEPLOY_PHYS_ADDR_START + *ppos;
  void *tmp_buf;

  if (len > KMALLOC_MAX_SIZE) {
    len = KMALLOC_MAX_SIZE;
  }
  
  if (*ppos + DEPLOY_PHYS_ADDR_START > DEPLOY_PHYS_ADDR_END) {
    len = 0;
  } else if (len + *ppos + DEPLOY_PHYS_ADDR_START > DEPLOY_PHYS_ADDR_END) {
    len = DEPLOY_PHYS_ADDR_END - *ppos - DEPLOY_PHYS_ADDR_START;
  }

  tmp_buf = kmalloc(len, GFP_KERNEL);
  
  io_addr = ioremap(addr, len);
  memcpy_fromio(tmp_buf, io_addr, len);
  iounmap(io_addr);
  
  len -= copy_to_user(buf, tmp_buf, len);
  kfree(tmp_buf);
  *ppos += len;
  return len;
}

static loff_t memory_llseek(struct file *file, loff_t offset, int origin) {
  switch (origin) {
  case SEEK_END:
    offset += DEPLOY_PHYS_ADDR_END - DEPLOY_PHYS_ADDR_START;
    break;
  case SEEK_CUR:
    offset += file->f_pos;
    break;
  }
  file->f_pos = offset;

  return offset;
}

static ssize_t zero_clear_read(struct file *file, char __user *buf, size_t len, loff_t *ppos) {
  if(*ppos != 0) {
    return 0;
  }
  snprintf(buf, len, "%u", zero_clear_status);
  *ppos = strlen(buf) + 1;
  return strlen(buf) + 1; 
}

static ssize_t zero_clear_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos) {
  unsigned int tmp;
  char buf_tmp[256];

  if(zero_clear_status != 0) {
    return len;
  }

  snprintf(buf_tmp, min(len+1, 256), "%s", buf);

  if(kstrtouint(buf_tmp, 10, &tmp) != 0) {
    return len;
  }

  if(tmp > (DEPLOY_PHYS_MEM_SIZE / PAGE_SIZE) ) {
    return len;
  }

  zero_clear_status = tmp;
  schedule_work(&zero_clear_workq);

  return 1;

}

void memory_zero_clear(struct work_struct *work) {
  void __iomem *io_addr;
  int i;

  for(i = 0; i < zero_clear_status; i++) {
    io_addr = ioremap(DEPLOY_PHYS_ADDR_START + PAGE_SIZE*i, PAGE_SIZE);
    memset(io_addr, 0, PAGE_SIZE);
    iounmap(io_addr);
  }

  zero_clear_status = 0;
  return;
}

static struct file_operations memory_fops = {
  .owner = THIS_MODULE,
  .read = memory_read,
  .llseek = memory_llseek,
};

static struct file_operations zero_clear_fops = {
  .owner = THIS_MODULE,
  .read = zero_clear_read,
  .write = zero_clear_write,
};

int __init debugmem_init(void) {
  topdir = debugfs_create_dir("friend_loader", NULL);
  if (!topdir) {
    return -ENOMEM;
  }
  
  memory = debugfs_create_file("memory", 0400, topdir, NULL, &memory_fops);
  if (!memory) {
    return -ENOMEM;
  }

  zero_clear = debugfs_create_file("zero_clear", 0400, topdir, NULL, &zero_clear_fops);
  if (!zero_clear) {
    return -ENOMEM;
  }

  INIT_WORK(&zero_clear_workq, memory_zero_clear);

  return 0;
}

void __exit debugmem_exit(void) {
  debugfs_remove_recursive(topdir);
}
