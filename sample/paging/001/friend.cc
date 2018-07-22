#include "type.h"
#include "common/_memory.h"
#include "channel/hakase.h"
#include "common/channel_accessor.h"

int16_t get_cpuid() {
  int16_t id;
  asm volatile("movw %%fs:0x4, %0" : "=r"(id));
  return id;
}

void puts(F2H &f2h, const char *str) {
  while(*str) {
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

  puts(f2h, "hello!");

  uint64_t *pml4t = (uint64_t *)0x310000UL;
  uint64_t *pdpt  = (uint64_t *)0x311000UL;
  uint64_t *pd1   = (uint64_t *)0x312000UL;
  uint64_t *pd2   = (uint64_t *)0x313000UL;

  pml4t[0] = ((uint64_t)pdpt + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[0]  = ((uint64_t)pd1 + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[3]  = ((uint64_t)pd2 + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  for(int i = 0; i < 512; i++) {
    pd1[i]  = (0x80000000UL + 0x200000UL * i) | (1 << 0) | (1 << 1) | (1 << 2) | (1<<7);
  }
  asm volatile("movq %0, %%cr3"::"r"((uint64_t)pml4t + 0x80000000UL));

  //!!!!!!!!!!!!!!!!!!!!!!
  // add your codes here!
  //!!!!!!!!!!!!!!!!!!!!!!

  puts(f2h, "bye!");

  return_value(f2h, 0);
  
  return 0;
}
