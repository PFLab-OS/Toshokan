#include <toshokan/friend/offload.h>
#include "shared.h"

int (*EXPORTED_SYMBOL(printf))(const char *format, ...);

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

extern Page SHARED_SYMBOL(__toshokan_pdpt);
Page pd;
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

  static const virt_addr_t vaddr = 0x80000000;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(vaddr % k512GB) / k1GB] =
      v2p((virt_addr_t)(&pd)) | (1 << 0) | (1 << 1) | (1 << 2);
  pd.entry[(vaddr % k1GB) / k2MB] =
      0x40000000 | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);

  uint64_t x = *((uint64_t *)vaddr);

  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("%lx: %lx\n", vaddr, x);
    EXPORTED_SYMBOL(printf)
    ("ready\n");
  });
  while (1) {
    asm volatile("hlt;");
  }
}
