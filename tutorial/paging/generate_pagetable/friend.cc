#include <toshokan/friend/offload.h>
#include "shared.h"

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

extern Page SHARED_SYMBOL(__toshokan_pdpt);
Page pml4t;
Page pdpt;
Page pd;
Page pt[512];
using virt_addr_t = uint64_t;
using phys_addr_t = uint64_t;

static inline phys_addr_t v2p(virt_addr_t vaddr) {
  // assume physical memory is mapped straight to virtual memory
  return vaddr;
}

void friend_main() {
  static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
  static const size_t k1GB = 1024UL * 1024 * 1024;
  static const size_t k2MB = 2UL * 1024 * 1024;
  static const size_t k4KB = 4UL * 1024;

  pml4t.entry[0] = v2p((virt_addr_t)&pdpt) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt.entry[1] = v2p((virt_addr_t)&pd) | (1 << 0) | (1 << 1) | (1 << 2);

  for(int i = 0; i < 512; i++) {
    pd.entry[i] = v2p((virt_addr_t)&pt[i]) | (1 << 0) | (1 << 1) | (1 << 2);
    for(int j = 0; j < 512; j++) {
      pt[i].entry[j] = (0x40000000 + i * 0x200000 + j * 0x1000) | (1 << 0) | (1 << 1) | (1 << 2);
    }
  }
  asm volatile("movq %0, %%cr3":: "r"(v2p((virt_addr_t)&pml4t)));
  uint64_t x = *((uint64_t *)0x40000000);

  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("0x%lx: %lx\n", 0x40000000L, x);
    EXPORTED_SYMBOL(printf)
    ("ready\n");
  });
}
