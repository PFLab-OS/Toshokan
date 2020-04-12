#include <stdint.h>
#include <toshokan/friend/offload.h>
#include "shared.h"

typedef uint64_t virt_addr;

volatile uint16_t _idtr[5];

struct idt_entity {
  uint32_t entry[4];
} __attribute__((aligned(8))) idt_def[64];

extern "C" void int_handler();

int wait_input() {
  int i = 0;
  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("1) divide without exception\n");
    EXPORTED_SYMBOL(printf)
    ("2) divide by 0 and raise exception\n");
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

  virt_addr vaddr = reinterpret_cast<virt_addr>(handler);
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
  setup_inthandler(int_handler);
  asm volatile("lidt (%0)" ::"r"(_idtr));
  switch (wait_input()) {
    case 1:
      asm volatile("divl %2" ::"a"(1), "d"(0), "b"(1));  // 1 / 1
      break;
    case 2:
      asm volatile("divl %2" ::"a"(1), "d"(0), "b"(0));  // 1 / 0
      break;
  }
}
