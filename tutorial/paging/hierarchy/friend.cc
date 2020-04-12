#include <toshokan/friend/offload.h>
#include "shared.h"

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

extern Page SHARED_SYMBOL(__toshokan_pdpt);
Page pd;
Page pt1, pt2;
using virt_addr_t = uint64_t;
using phys_addr_t = uint64_t;

static inline phys_addr_t v2p(virt_addr_t vaddr) {
  // assume physical memory is mapped straight to virtual memory
  return vaddr;
}

void wait_input(int i) {
  OFFLOAD({
    int c;
    EXPORTED_SYMBOL(printf)
    ("[%d/2] press Enter key to go to the next step:", i);
    do {
      c = EXPORTED_SYMBOL(getchar)();
    } while (c != '\n');
  });
}

void friend_main() {
  static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
  static const size_t k1GB = 1024UL * 1024 * 1024;
  static const size_t k2MB = 2UL * 1024 * 1024;
  static const size_t k4KB = 4UL * 1024;

  static const virt_addr_t vaddr1 = 0x80000000;
  static const virt_addr_t vaddr2 = 0x80200000;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(vaddr1 % k512GB) / k1GB] =
      v2p((virt_addr_t)(&pd)) | (1 << 0) | (1 << 1) | (1 << 2);
  pd.entry[(vaddr1 % k1GB) / k2MB] =
      v2p((virt_addr_t)(&pt1)) | (1 << 0) | (1 << 1) | (1 << 2);

  // setup a page
  pt1.entry[(vaddr1 % k2MB) / k4KB] =
      0x100000000UL | (1 << 0) | (1 << 1) | (1 << 2);

  wait_input(1);

  // to setup page for vaddr2, we have to prepare another page table.
  pd.entry[(vaddr2 % k1GB) / k2MB] =
      v2p((virt_addr_t)(&pt2)) | (1 << 0) | (1 << 1) | (1 << 2);
  pt2.entry[(vaddr2 % k2MB) / k4KB] =
      0x100000000UL | (1 << 0) | (1 << 1) | (1 << 2);

  wait_input(2);
}
