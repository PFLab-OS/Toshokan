#include "common/_memory.h"
#include "common/channel.h"
#include "common/channel_accessor.h"
#include "type.h"

void puts(F2H &f2h, const char *str) {
  while (*str) {
    ChannelAccessor<1> ch_ac(f2h, 2);
    ch_ac.Write<char>(0, *str);
    ch_ac.Do(0);
    str++;
  }
  ChannelAccessor<1> ch_ac(f2h, 2);
  ch_ac.Write<char>(0, '\n');
  ch_ac.Do(0);
}

void return_value(F2H &f2h, int i) {
  ChannelAccessor<sizeof(int)> ch_ac(f2h, 1);
  ch_ac.Write<int>(0, i);
  ch_ac.Do(0);
}

int main() {
  F2H f2h;

  puts(f2h, "booted");

  uint64_t *pml4t = (uint64_t *)0x301000UL;
  uint64_t *pdpt = (uint64_t *)0x302000UL;
  uint64_t *pd = (uint64_t *)0x303000UL;

  for (int i = 0; i < 512; i++) {
    pml4t[i] = 0;
    pdpt[i] = 0;
    pd[i] = 0;
  }

  pml4t[0] = ((uint64_t)pdpt + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[0] =
      ((uint64_t)pd + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);
  pd[0] = 0x80000000UL | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);
  pd[1] = 0x80200000UL | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);
  //!!!!!!!!!!!!!!!!!!!!!!
  // add your codes here!
  //!!!!!!!!!!!!!!!!!!!!!!

  asm volatile("movq %0, %%cr3" ::"r"((uint64_t)pml4t + 0x80000000UL));

  if (*((uint64_t *)0x200000UL) == *((uint64_t *)0x40000000UL)) {
    return_value(f2h, 0);
  } else {
    return_value(f2h, 1);
  }

  return 0;
}
