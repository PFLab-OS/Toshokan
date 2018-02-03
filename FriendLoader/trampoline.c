#include <asm/realmode.h>
#include "trampoline.h"

static const size_t buf_size = 0x1000;

int trampoline_region_alloc(struct trampoline_region *region) {
  dma_addr_t tpaddr;
  for (tpaddr = 0x1000; tpaddr < 0x100000; tpaddr += buf_size) {
    uint32_t* io_addr = ioremap(tpaddr, buf_size);
    if (io_addr == 0) {
      continue;
    }
    if (io_addr[1] == FRIEND_LOADER_TRAMPOLINE_SIGNATURE) {
      region->paddr = tpaddr;
      region->vaddr = io_addr;
      return 0;
    }
    iounmap(io_addr);
  }
  return -1;
}

void trampoline_region_free(struct trampoline_region *region) {
  iounmap(region->vaddr);
}
