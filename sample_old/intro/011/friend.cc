#include "channel/hakase.h"
#include "common/_memory.h"
#include "common/channel_accessor.h"
#include "type.h"

int16_t get_cpuid() {
  int16_t id;
  asm volatile("movw %%fs:0x4, %0" : "=r"(id));
  return id;
}

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

  puts(f2h, "hello!");

  //!!!!!!!!!!!!!!!!!!!!!!
  // add your codes here!
  //!!!!!!!!!!!!!!!!!!!!!!
  asm volatile("hlt;");

  puts(f2h, "bye!");

  return_value(f2h, 0);

  return 0;
}
