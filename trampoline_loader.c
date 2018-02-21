#include <asm/realmode.h>
#include "trampoline_loader.h"

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

static uint8_t jmp_bin[] = {0xeb, 0x06, 0x66, 0x90};

int trampoline_region_init(struct trampoline_region *region, uint8_t *bin, size_t bin_size) {
  if (bin_size >= 0x4088) {
    return -1;
  }

  memcpy(region->vaddr, jmp_bin, 4);
  
  memcpy(region->vaddr + 2, bin, bin_size);

  return 0;
}

void trampoline_region_free(struct trampoline_region *region) {
  iounmap(region->vaddr);
}
