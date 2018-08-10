#include "app.h"
#include "common.h"

void show_i(F2H &f2h, int i) {
  char mes[] = "i=0";
  mes[2] += i;
  puts(f2h, mes);
}

int main() {
  F2H f2h;

  puts(f2h, "hello!");

  memset((void *)0x310000UL, 0, 0x5000);

  uint64_t *pml4t = (uint64_t *)0x310000UL;
  uint64_t *pdpt = (uint64_t *)0x311000UL;
  uint64_t *pd1 = (uint64_t *)0x312000UL;
  uint64_t *pd2 = (uint64_t *)0x313000UL;
  uint64_t *pt1 = (uint64_t *)0x314000UL;

  pml4t[0] = ((uint64_t)pdpt + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[0] = ((uint64_t)pd1 + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[3] = ((uint64_t)pd2 + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pd2[0] = ((uint64_t)pt1 + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  for (int i = 0; i < 512; i++) {
    pd1[i] = (0x80000000UL + 0x200000UL * i) | (1 << 0) | (1 << 1) | (1 << 2) |
             (1 << 7);
  }
  asm volatile("movq %0, %%cr3" ::"r"((uint64_t)pml4t + 0x80000000UL));

  extern uint8_t _binary_app1_start[];
  extern uint8_t _binary_app1_size[];
  extern uint8_t _binary_app2_start[];
  extern uint8_t _binary_app2_size[];
  memcpy((void *)0x500000UL, (const void *)_binary_app1_start,
         (size_t)_binary_app1_size);
  memcpy((void *)0x600000UL, (const void *)_binary_app2_start,
         (size_t)_binary_app2_size);

  Context c1;
  c1.next = (Context(*)(Context))0xC0000000UL;  // entry1
  c1.i = 0;

  pt1[0] = 0x80500000UL | (1 << 0) | (1 << 1) | (1 << 2);
  asm volatile("invlpg (%0)" ::"b"(0xC0000000UL) : "memory");
  pt1[1] = 0x80700000UL | (1 << 0) | (1 << 1) | (1 << 2);
  asm volatile("invlpg (%0)" ::"b"(0xC0001000UL) : "memory");
  c1 = c1.next(c1);  // call entry1

  show_i(f2h, c1.i);

  if (c1.i != 1) {
    return_value(f2h, 1);
    return 1;
  }

  Context c2;
  c2.next = (Context(*)(Context))0xC0000000UL;  // entry3
  c2.i = 0;

  pt1[0] = 0x80600000UL | (1 << 0) | (1 << 1) | (1 << 2);
  asm volatile("invlpg (%0)" ::"b"(0xC0000000UL) : "memory");
  pt1[1] = 0;
  asm volatile("invlpg (%0)" ::"b"(0xC0001000UL) : "memory");
  c2 = c2.next(c2);  // call entry3

  show_i(f2h, c2.i);

  if (c2.i != 9) {
    return_value(f2h, 1);
    return 1;
  }

  pt1[0] = 0x80500000UL | (1 << 0) | (1 << 1) | (1 << 2);
  asm volatile("invlpg (%0)" ::"b"(0xC0000000UL) : "memory");
  pt1[1] = 0;
  asm volatile("invlpg (%0)" ::"b"(0xC0001000UL) : "memory");
  c1 = c1.next(c1);  // call entry2

  show_i(f2h, c1.i);

  if (c1.i != 2) {
    return_value(f2h, 1);
    return 1;
  }

  pt1[0] = 0x80600000UL | (1 << 0) | (1 << 1) | (1 << 2);
  asm volatile("invlpg (%0)" ::"b"(0xC0000000UL) : "memory");
  pt1[1] = 0x80700000UL | (1 << 0) | (1 << 1) | (1 << 2);
  asm volatile("invlpg (%0)" ::"b"(0xC0001000UL) : "memory");
  c2 = c2.next(c2);  // call entry4

  show_i(f2h, c2.i);

  if (c2.i != 8) {
    return_value(f2h, 1);
    return 1;
  }

  puts(f2h, "bye!");

  return_value(f2h, 0);

  return 0;
}
