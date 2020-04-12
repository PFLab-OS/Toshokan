#include <toshokan/friend/offload.h>
#include "shared.h"

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

extern Page SHARED_SYMBOL(__toshokan_pdpt);
Page pd;
Page pt;
using virt_addr_t = uint64_t;
using phys_addr_t = uint64_t;

static inline phys_addr_t v2p(virt_addr_t vaddr) {
  // assume physical memory is mapped straight to virtual memory
  return vaddr;
}

void show_memdata(virt_addr_t vaddr) {
  uint64_t x = *((uint64_t *)vaddr);
  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("0x%lx: %lx\n", vaddr, x);
  });
}

void wait_input(int i) {
  OFFLOAD({
    int c;
    EXPORTED_SYMBOL(printf)
    ("[%d/3] press Enter key to go to the next step:", i);
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
  static const virt_addr_t vaddr2 = 0x80001000;
  static const virt_addr_t vaddr3 = 0x80002000;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(vaddr1 % k512GB) / k1GB] =
      v2p((virt_addr_t)(&pd)) | (1 << 0) | (1 << 1) | (1 << 2);
  pd.entry[(vaddr1 % k1GB) / k2MB] =
      v2p((virt_addr_t)(&pt)) | (1 << 0) | (1 << 1) | (1 << 2);

  // setup a page
  pt.entry[(vaddr1 % k2MB) / k4KB] =
      0x100000000UL | (1 << 0) | (1 << 1) | (1 << 2);

  show_memdata(vaddr1);
  wait_input(1);

  // remap the page
  pt.entry[(vaddr1 % k2MB) / k4KB] =
      0x100001000UL | (1 << 0) | (1 << 1) | (1 << 2);

  // flush TLB
  asm volatile("invlpg (%0)" ::"r"(vaddr1) : "memory");

  show_memdata(vaddr1);
  wait_input(2);

  // setup other pages
  pt.entry[(vaddr2 % k2MB) / k4KB] =
      0x100000000UL | (1 << 0) | (1 << 1) | (1 << 2);
  pt.entry[(vaddr3 % k2MB) / k4KB] =
      0x100001000UL | (1 << 0) | (1 << 1) | (1 << 2);

  show_memdata(vaddr2);
  show_memdata(vaddr3);
  wait_input(3);
}
