#include <asm/io.h>
#include <linux/kernel.h>

#include "common.h"
#include "deploy_data.h"

struct deployment_info {
    phys_addr_t phys_addr_start;
    phys_addr_t phys_addr_end;
};

static void write_phys_mem(phys_addr_t addr, const void* data, size_t size);

int deploy_data(const char* data, size_t size, loff_t offset)
{
  // FIXME: 0x80000000から0x80000000だけ空いている物と仮定している。
  struct deployment_info di = {
    .phys_addr_start = 0x80000000,
    .phys_addr_end =  0x100000000,
  };
  
  if (di.phys_addr_start + offset + size > di.phys_addr_end) {
    return 1;
  }

  write_phys_mem(di.phys_addr_start + offset, data, size);
  pr_info("depftom_init: "
	  "deployed to physical memory [%llx - %llx]\n",
	  di.phys_addr_start + offset,
	  di.phys_addr_start + offset + size);

  return 0;
}

static void write_phys_mem(phys_addr_t addr, const void* data, size_t size)
{
  void __iomem* io_addr = ioremap(addr, size);
  memcpy_toio(io_addr, data, size);
  iounmap(io_addr);
}
