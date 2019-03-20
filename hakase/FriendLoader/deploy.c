#include <asm/io.h>
#include <linux/kernel.h>

#include "deploy.h"
#include "_memory.h"

int deploy(const char *data, size_t size, loff_t offset) {
  void __iomem *io_addr;
  loff_t offset_ = (offset / 0x1000) * 0x1000;
  phys_addr_t addr = DEPLOY_PHYS_ADDR_START + offset_;
  if (addr + size + (offset % 0x1000) > DEPLOY_PHYS_ADDR_END) {
    pr_warn("friend_loader: file size too large\n");
    return -1;
  }

  io_addr = ioremap(addr, ((size + 0x1000 - 1) / 0x1000) * 0x1000);
  if (io_addr == 0) {
    pr_err("friend_loader: failed to remap address.(addr: %llx, size: %lx)", addr, size);
    return -1;
  }
  memcpy_toio(io_addr + (offset % 0x1000), data, size);
  iounmap(io_addr);

  return 0;
}

int read_deploy_area(char *buf, size_t size, loff_t offset) {
  void __iomem *io_addr;
  loff_t offset_ = (offset / 0x1000) * 0x1000;
  phys_addr_t addr = DEPLOY_PHYS_ADDR_START + offset_;
  if (addr + size + (offset % 0x1000) > DEPLOY_PHYS_ADDR_END) {
    pr_warn("friend_loader: file size too large\n");
    return -1;
  }

  io_addr = ioremap(addr, ((size + 0x1000 - 1) / 0x1000) * 0x1000);
  if (io_addr == 0) {
    pr_err("friend_loader: failed to remap address.(addr: %llx, size: %lx)", addr, size);
    return -1;
  }
  memcpy_fromio(buf, io_addr + (offset % 0x1000), size);
  iounmap(io_addr);

  return 0;
}
int deploy_zero(loff_t offset, size_t size) {
  void __iomem *io_addr;
  phys_addr_t addr = DEPLOY_PHYS_ADDR_START + offset;
  if (addr + size > DEPLOY_PHYS_ADDR_END) {
    pr_warn("friend_loader: file size too large\n");
    return -1;
  }

  io_addr = ioremap(addr, size);
  if (io_addr == 0) {
    pr_err("friend_loader: failed to remap address.(addr: %llx, size: %lx)", addr, size);
    return -1;
  }
  memset_io(io_addr, 0, size);
  iounmap(io_addr);

  return 0;
}
