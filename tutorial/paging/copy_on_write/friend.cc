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

static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
static const size_t k1GB = 1024UL * 1024 * 1024;
static const size_t k2MB = 2UL * 1024 * 1024;
static const size_t k4KB = 4UL * 1024;

static inline void my_memcpy(void *dst, void *src, size_t n) {
  uint64_t *dst_ = (uint64_t *)dst;
  uint64_t *src_ = (uint64_t *)src;
  for (size_t i = 0; i < n / sizeof(uint64_t); i += sizeof(uint64_t)) {
    dst_[i] = src_[i];
  }
}

extern "C" void fault_handler() {
  uint64_t fault_address;
  asm volatile("movq %%cr2, %0;" : "=r"(fault_address));
  // we assume that fault address is managed by 'pt'.
  // and the entry is higher than 256.
  int fault_entry = (fault_address % k2MB) / k4KB;
  pt.entry[fault_entry] =
      (0x50000000 + fault_entry * 0x1000) | (1 << 0) | (1 << 1) | (1 << 2);
  int src_entry = fault_entry - 256;
  pt.entry[src_entry] |= (1 << 1);
  my_memcpy((void *)(0x50000000 + fault_entry * 0x1000),
            (void *)(0x50000000 + src_entry * 0x1000), 0x1000);
  asm volatile("invlpg (%0)" ::"r"(fault_address) : "memory");
}

void friend_main() {
  setup_inthandler(int_handler);
  asm volatile("lidt (%0)" ::"r"(_idtr));

  static const virt_addr_t vaddr1 = 0x80000000;
  static const virt_addr_t vaddr2 = 0x80100000;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(vaddr1 % k512GB) / k1GB] =
      v2p((virt_addr_t)(&pd)) | (1 << 0) | (1 << 1) | (1 << 2);
  pd.entry[(vaddr1 % k1GB) / k2MB] =
      v2p((virt_addr_t)(&pt)) | (1 << 0) | (1 << 1) | (1 << 2);
  for (int i = 0; i < 256; i++) {
    pt.entry[i] = (0x50000000 + i * 0x1000) | (1 << 0) | (1 << 1) | (1 << 2);
  }

  {
    uint64_t t1 = rdtscp();
    for (int i = 256; i < 512; i++) {
      pt.entry[i] = (0x50000000 + i * 0x1000) | (1 << 0) | (1 << 1) | (1 << 2);
    }
    my_memcpy((void *)vaddr2, (void *)vaddr1, 0x100000);
    for (int i = 0; i < 256; i++) {
      *((uint64_t *)(0x80100000 + i * 0x1000)) = 0;
    }
    uint64_t t2 = rdtscp();
    OFFLOAD({ EXPORTED_SYMBOL(printf)("%d\n", t2 - t1); });
  }

  {
    uint64_t t1 = rdtscp();
    for (int i = 0; i < 256; i++) {
      pt.entry[i] = (0x50000000 + i * 0x1000) | (1 << 0) | (1 << 2);
    }
    for (int i = 256; i < 512; i++) {
      pt.entry[i] = pt.entry[i - 256];
    }
    for (int i = 0; i < 256; i++) {
      *((uint64_t *)(0x80100000 + i * 0x1000)) = 0;
    }
    uint64_t t2 = rdtscp();
    OFFLOAD({ EXPORTED_SYMBOL(printf)("%d\n", t2 - t1); });
  }
}
