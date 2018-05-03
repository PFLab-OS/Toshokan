#include <asm/io.h>
#include <linux/kernel.h>

#include "common.h"
#include "deploy.h"

int deploy(const char *data, size_t size, loff_t offset) {
  void __iomem *io_addr;
  phys_addr_t addr = DEPLOY_PHYS_ADDR_START + offset;
  if (addr + size > DEPLOY_PHYS_ADDR_END) {
    pr_warn("deploy: file size too large\n");
    return -1;
  }

  io_addr = ioremap(addr, size);
  memcpy_toio(io_addr, data, size);
  iounmap(io_addr);

  return 0;
}

int deploy_zero(size_t size, loff_t offset) {
  void __iomem *io_addr;
  phys_addr_t addr = DEPLOY_PHYS_ADDR_START + offset;
  if (addr + size > DEPLOY_PHYS_ADDR_END) {
    pr_warn("deploy: file size too large\n");
    return -1;
  }

  io_addr = ioremap(addr, size);
  memset_io(io_addr, 0, size);
  iounmap(io_addr);

  return 0;
}
