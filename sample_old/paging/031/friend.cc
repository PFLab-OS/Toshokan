#include "common.h"

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

  for (int i = 0; i < 1024; i++) {
    pt1[i] = (0x80500000UL + 0x1000UL * i) | (1 << 0) | (1 << 1) | (1 << 2);
  }

  *((int *)0xC0000000UL) = 0;

  puts(f2h, "bye!");

  return_value(f2h, 0);

  return 0;
}
