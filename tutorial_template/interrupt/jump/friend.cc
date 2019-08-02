#include <stdint.h>
#include "shared.h"

typedef uint64_t virt_addr;

volatile uint16_t _idtr[5];

struct idt_entity {
  uint32_t entry[4];
} __attribute__((aligned(8))) idt_def[64];

extern "C" void int_handler();

void wait_input(int i) {
  OFFLOAD({
    int i;
    EXPORTED_SYMBOL(printf)
    ("1) just do division by 0\n");
    EXPORTED_SYMBOL(printf)
    ("2) retry without doing anything\n");
    EXPORTED_SYMBOL(printf)
    ("3) recover from 0div exception\n");
    do {
    EXPORTED_SYMBOL(printf)
    ("choose one sample [1-3]:");
    i = atoi(EXPORTED_SYMBOL(getchar)());
    } while (i < 0 || i > 3);
  });
}


void setup_inthandler() {
  // do not try to understand this.
  // it's off-topic at this sample.

  virt_addr vaddr = reinterpret_cast<virt_addr>(int_handler);
  idt_def[0].entry[0] = (vaddr & 0xFFFF) | (0x10 << 16);
  idt_def[0].entry[1] = (vaddr & 0xFFFF0000) | (0xE << 8) | (1 << 15);
  idt_def[0].entry[2] = vaddr >> 32;
  idt_def[0].entry[3] = 0;

  virt_addr idt_addr = reinterpret_cast<virt_addr>(idt_def);
  _idtr[0] = 0x10 * 64 - 1;
  _idtr[1] = idt_addr & 0xffff;
  _idtr[2] = (idt_addr >> 16) & 0xffff;
  _idtr[3] = (idt_addr >> 32) & 0xffff;
  _idtr[4] = (idt_addr >> 48) & 0xffff;
}

void friend_main() {
  setup_inthandler();
  asm volatile("lidt (%0)" ::"r"(_idtr));
  volatile int i = 3 / 0;
}
