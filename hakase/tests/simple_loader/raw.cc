#include <stdint.h>
#include "common/_memory.h"

int main() __attribute__((section(".text.boot")));

int main() {
  unsigned int *channel = reinterpret_cast<unsigned int *>(MemoryMap::kF2h);
  int16_t id;
  asm volatile("movw %%fs:0x0, %0" : "=r"(id));
  channel[0] = 1 | (id << 16);
  asm volatile("cli;hlt;hlt;");
  return 0;
}
