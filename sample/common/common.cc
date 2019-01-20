#include "common.h"

int16_t get_cpuid() {
  int16_t id;
  asm volatile("movw %%fs:0x4, %0" : "=r"(id));
  return id;
}

static int print_flag = 0;
void puts(F2H &f2h, const char *str) {
  while(__sync_bool_compare_and_swap(&print_flag, 0, 1)) {
    asm volatile("pause");
  }
  while (*str) {
    ChannelAccessor<1> ch_ac(f2h, 2);
    ch_ac.Write<char>(0, *str);
    ch_ac.Do(0);
    str++;
  }
  ChannelAccessor<1> ch_ac(f2h, 2);
  ch_ac.Write<char>(0, '\n');
  ch_ac.Do(0);
  print_flag = 0;
}

void return_value(F2H &f2h, int i) {
  ChannelAccessor<sizeof(int)> ch_ac(f2h, 1);
  ch_ac.Write<int>(0, i);
  ch_ac.Do(0);
}

void memset(void *dest, int ch, size_t n) {
  for (size_t i = 0; i < n; i++) {
    ((uint8_t *)dest)[i] = ch;
  }
}

void memcpy(void *dest, const void *src, size_t n) {
  for (size_t i = 0; i < n; i++) {
    ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
  }
}
