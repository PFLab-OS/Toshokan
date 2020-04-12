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

  // write your code here

  asm volatile("movq %0, %%cr3" ::"r"(v2p((virt_addr_t)&pml4t)));
  uint64_t x = *((uint64_t *)0x40000000);

  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("0x%lx: %lx\n", 0x40000000L, x);
  });
}
