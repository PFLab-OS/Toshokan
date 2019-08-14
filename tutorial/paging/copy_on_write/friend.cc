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

volatile uint16_t _idtr[5];

struct idt_entity {
  uint32_t entry[4];
} __attribute__((aligned(8))) idt_def[64];

extern "C" void int_handler();

void setup_inthandler(void (*handler)()) {
  // do not try to understand this.
  // it's off-topic at this sample.

  virt_addr_t vaddr = reinterpret_cast<virt_addr_t>(handler);
  idt_def[14].entry[0] = (vaddr & 0xFFFF) | (0x10 << 16);
  idt_def[14].entry[1] = (vaddr & 0xFFFF0000) | (0xE << 8) | (1 << 15);
  idt_def[14].entry[2] = vaddr >> 32;
  idt_def[14].entry[3] = 0;

  virt_addr_t idt_addr = reinterpret_cast<virt_addr_t>(idt_def);
  _idtr[0] = 0x10 * 64 - 1;
  _idtr[1] = idt_addr & 0xffff;
  _idtr[2] = (idt_addr >> 16) & 0xffff;
  _idtr[3] = (idt_addr >> 32) & 0xffff;
  _idtr[4] = (idt_addr >> 48) & 0xffff;
}

static inline uint64_t rdtscp() {
  uint32_t aux;
  uint64_t rax, rdx;
  asm volatile("rdtscp\n" : "=a"(rax), "=d"(rdx), "=c"(aux));
  return (rdx << 32) + rax;
}

static inline void my_memcpy(void *dst, void *src, size_t n) {}

extern "C" void fault_handler() {}

void friend_main() {
  setup_inthandler(int_handler);
  asm volatile("lidt (%0)" ::"r"(_idtr));

  static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
  static const size_t k1GB = 1024UL * 1024 * 1024;
  static const size_t k2MB = 2UL * 1024 * 1024;
  static const size_t k4KB = 4UL * 1024;

  static const virt_addr_t vaddr1 = 0x80000000;
  static const virt_addr_t vaddr2 = 0x80100000;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(vaddr1 % k512GB) / k1GB] =
      v2p((virt_addr_t)(&pd)) | (1 << 0) | (1 << 1) | (1 << 2);
  pd.entry[(vaddr1 % k1GB) / k2MB] =
      v2p((virt_addr_t)(&pt)) | (1 << 0) | (1 << 1) | (1 << 2);
  for (int i = 0; i < 256; i++) {
    pt.entry[(vaddr1 % k2MB) / k4KB] =
        (0x50000000 + i * 0x1000) | (1 << 0) | (1 << 1) | (1 << 2);
  }

  my_memcpy(vaddr2, vaddr1, 0x100000);
}
