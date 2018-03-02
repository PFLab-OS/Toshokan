#include <asm/realmode.h>
#include "common.h"
#include "trampoline_loader.h"

/*
 * format of trampoline region
 * 
 * region_offset + 0x00: jmp 0x20
 * region_offset + 0x04: FRIEND_LOADER_TRAMPOLINE_SIGNATURE
 * region_offset + 0x08: region_offset
 * region_offset + 0x10: phys_addr_start
 * region_offset + 0x18: phys_addr_end
 * region_offset + 0x20: entry of trampoline bin
 * region_offset + 0x1000: end of trampoline region
 *
 * trampoline binary should be loaded at "region_offset + 0x08".
 */

static uint8_t jmp_bin[] = {0xeb, 0x1e, 0x66, 0x90}; // jmp 0x20; xchg %ax, &ax
static const size_t buf_size = 0x1000;

int trampoline_region_alloc(struct trampoline_region *region) {
  dma_addr_t tpaddr;
  uint32_t* io_addr;
  
  // search signature
  for (tpaddr = 0x1000; tpaddr < 0x100000; tpaddr += buf_size) {
    if (tpaddr >= 0xA0000 && tpaddr <= 0xBF000) {
      // this area is reserved. (ref. Multiprocessor Specification)
      continue;
    }
    io_addr = ioremap(tpaddr, buf_size);
    if (io_addr == 0) {
      continue;
    }
    if (io_addr[1] == FRIEND_LOADER_TRAMPOLINE_SIGNATURE) {
      // found
      region->paddr = tpaddr;
      region->vaddr = io_addr;
      return 0;
    }
    iounmap(io_addr);
  }
  return -1;
}

int trampoline_region_init(struct trampoline_region *region, dma_addr_t phys_addr_start, dma_addr_t phys_addr_end) {
  extern uint8_t _binary_trampoline_bin_start[];
  extern uint8_t _binary_trampoline_bin_end[];
  extern uint8_t _binary_trampoline_bin_size[];
  size_t binary_trampoline_bin_size = (size_t)_binary_trampoline_bin_size;

  uint64_t *vaddr64 = (uint64_t *)region->vaddr;
  
  memcpy(region->vaddr, jmp_bin, sizeof(jmp_bin) / sizeof(jmp_bin[0]));

  if (buf_size - 8 < binary_trampoline_bin_size) {
    return -1;
  }

  if (_binary_trampoline_bin_start + binary_trampoline_bin_size
      != _binary_trampoline_bin_end) {
    // invalid state
    return -1;
  }

  // copy trampoline binary to trampoline region + 8 byte
  memcpy(region->vaddr + 8 / (sizeof(*region->vaddr)), _binary_trampoline_bin_start, binary_trampoline_bin_size);

  // initialize trampoline header
  vaddr64[1] = region->paddr;
  vaddr64[2] = DEPLOY_PHYS_ADDR_START;
  vaddr64[3] = DEPLOY_PHYS_ADDR_END;

  return 0;
}

void trampoline_region_free(struct trampoline_region *region) {
  iounmap(region->vaddr);
}
