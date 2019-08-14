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

static inline uint64_t rdtscp() {
  uint32_t aux;
  uint64_t rax, rdx;
  asm volatile("rdtscp\n" : "=a"(rax), "=d"(rdx), "=c"(aux));
  return (rdx << 32) + rax;
}

void friend_main() {
  static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
  static const size_t k1GB = 1024UL * 1024 * 1024;
  static const size_t k2MB = 2UL * 1024 * 1024;
  static const size_t k4KB = 4UL * 1024;

  static const virt_addr_t vaddr1 = 0x80000000;
  static const virt_addr_t vaddr2 = 0x80001000;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(vaddr1 % k512GB) / k1GB] =
      v2p((virt_addr_t)(&pd)) | (1 << 0) | (1 << 1) | (1 << 2);
  pd.entry[(vaddr1 % k1GB) / k2MB] =
      v2p((virt_addr_t)(&pt)) | (1 << 0) | (1 << 1) | (1 << 2);
  pt.entry[(vaddr1 % k2MB) / k4KB] =
      0x40000000 | (1 << 0) | (1 << 1) | (1 << 2);
}
