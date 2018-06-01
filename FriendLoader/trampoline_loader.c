#include "trampoline_loader.h"
#include "deploy.h"
#include "memory.h"
#include <asm/realmode.h>

static uint8_t jmp_bin[] = {0xeb, 0x1e, 0x66, 0x90}; // jmp 0x20; xchg %ax, &ax
static const size_t trampoline_buf_align = 0x1000;
static const size_t trampoline_buf_size = 0x4000;

int trampoline_region_alloc(struct trampoline_region *region) {
  phys_addr_t tpaddr;
  uint32_t *io_addr;

  // search signature
  for (tpaddr = 0x1000; tpaddr < 0x100000; tpaddr += trampoline_buf_align) {
    if (tpaddr >= 0xA0000 && tpaddr <= 0xBF000) {
      // this area is reserved. (ref. Multiprocessor Specification)
      continue;
    }
    io_addr = __va(tpaddr);
    if (io_addr == 0) {
      continue;
    }
    if (io_addr[kMemoryMapSignature / sizeof(*io_addr)] == FRIEND_LOADER_TRAMPOLINE_SIGNATURE) {
      // found
      region->paddr = tpaddr;
      region->vaddr = io_addr;
      return 0;
    }
  }
  return -1;
}

int trampoline_region_init(struct trampoline_region *region,
                           phys_addr_t phys_addr_start,
                           phys_addr_t phys_addr_end) {
  extern uint8_t _binary_boot_trampoline_bin_start[];
  extern uint8_t _binary_boot_trampoline_bin_end[];
  extern uint8_t _binary_boot_trampoline_bin_size[];
  size_t binary_boot_trampoline_bin_size =
      (size_t)_binary_boot_trampoline_bin_size;

  uint64_t *vaddr64 = (uint64_t *)region->vaddr;

  memcpy(region->vaddr, jmp_bin, sizeof(jmp_bin) / sizeof(jmp_bin[0]));

  if (0x1000 - kMemoryMapTrampolineBinLoadPoint < binary_boot_trampoline_bin_size) {
    return -1;
  }

  if (_binary_boot_trampoline_bin_start + binary_boot_trampoline_bin_size !=
      _binary_boot_trampoline_bin_end) {
    // invalid state
    return -1;
  }

  // copy trampoline binary to trampoline region + 8 byte
  memcpy(region->vaddr + kMemoryMapTrampolineBinLoadPoint / (sizeof(*region->vaddr)),
         _binary_boot_trampoline_bin_start, binary_boot_trampoline_bin_size);

  // check address
  if ((phys_addr_start & (1 * 1024 * 1024 * 1024 - 1)) != 0) {
    // should be aligned to 1GB boundary
    // because of using 1GB huge page
    return -1;
  }

  // initialize trampoline header
  vaddr64[kMemoryMapRegionOffset / sizeof(*vaddr64)] = region->paddr;
  vaddr64[kMemoryMapPhysAddrStart / sizeof(*vaddr64)] = phys_addr_start;
  vaddr64[kMemoryMapReserved1 / sizeof(*vaddr64)] = 0;

  // make copy of trampoline region at deploy area
  deploy((const char *)region->vaddr, binary_boot_trampoline_bin_size + kMemoryMapTrampolineBinLoadPoint,
         region->paddr);
  deploy((const char *)region->vaddr, binary_boot_trampoline_bin_size + kMemoryMapTrampolineBinLoadPoint, 0);

  deploy_zero(kMemoryMapPml4t, kMemoryMapEnd - kMemoryMapPml4t);

  return 0;
}

void trampoline_region_free(struct trampoline_region *region) {
  iounmap(region->vaddr);
}
