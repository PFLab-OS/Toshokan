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

extern "C" void int_handler1();
extern "C" void int_handler2();

int wait_input() {
  int i = 0;
  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("1) retry without doing anything\n");
    EXPORTED_SYMBOL(printf)
    ("2) recover from 0div exception\n");
    do {
      EXPORTED_SYMBOL(printf)
      ("choose one sample [1-2]:");
      char buf[2];
      buf[0] = EXPORTED_SYMBOL(getchar)();
      buf[1] = '\0';
      i = EXPORTED_SYMBOL(atoi)(buf);
    } while (i < 1 || i > 2);
  });
  return i;
}

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

uint64_t *pt_entry;  // for int.S

void friend_main() {
  void (*int_handlers[])() = {
      int_handler1,
      int_handler2,
  };
  int input = wait_input();
  setup_inthandler(int_handlers[input - 1]);
  asm volatile("lidt (%0)" ::"r"(_idtr));

  static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
  static const size_t k1GB = 1024UL * 1024 * 1024;
  static const size_t k2MB = 2UL * 1024 * 1024;
  static const size_t k4KB = 4UL * 1024;

  static const virt_addr_t vaddr = 0x80000000;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(vaddr % k512GB) / k1GB] =
      v2p((virt_addr_t)(&pd)) | (1 << 0) | (1 << 1) | (1 << 2);
  pd.entry[(vaddr % k1GB) / k2MB] =
      v2p((virt_addr_t)(&pt)) | (1 << 0) | (1 << 1) | (1 << 2);

  // clear page table entry on purpose
  pt.entry[(vaddr % k2MB) / k4KB] = 0;
  pt_entry = &pt.entry[(vaddr % k2MB) / k4KB];  // set page table entry address

  uint64_t x = *((uint64_t *)vaddr);  // page fault may happen

  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("0x%lx: %lx\n", vaddr, x);
  });
}
