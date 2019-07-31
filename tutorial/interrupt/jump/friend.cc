#include <stdint.h>
#include "shared.h"

// do not try to understand this.
// it's off-topic at this sample.

typedef uint64_t virt_addr;

#define KERNEL_CS (0x10)
#define KERNEL_DS (0x18)
#define USER_DS (0x23)
#define USER_CS (0x2B)

volatile uint16_t _idtr[5];

struct idt_entity {
  uint32_t entry[4];
} __attribute__((aligned(8))) idt_def[1];

void dummy() {}

void SetupGeneric() {
  virt_addr vaddr = reinterpret_cast<virt_addr>(dummy);
  idt_def[0].entry[0] = (vaddr & 0xFFFF) | (KERNEL_CS << 16);
  idt_def[0].entry[1] = (vaddr & 0xFFFF0000) | (0xE << 8) | (1 << 15);
  idt_def[0].entry[2] = vaddr >> 32;
  idt_def[0].entry[3] = 0;

  virt_addr idt_addr = reinterpret_cast<virt_addr>(idt_def);
  _idtr[0] = 0x10 * 1 - 1;
  _idtr[1] = idt_addr & 0xffff;
  _idtr[2] = (idt_addr >> 16) & 0xffff;
  _idtr[3] = (idt_addr >> 32) & 0xffff;
  _idtr[4] = (idt_addr >> 48) & 0xffff;
}

void friend_main() {
  // asm volatile("lidt (%0)" ::"r"(_idtr));
  // asm volatile("sti;");
}
