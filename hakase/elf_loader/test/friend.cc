#include <stdint.h>
#include "common/_memory.h"

int buf1[10]; // for .bss
int buf2[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; // for .data

int main();

int main() {
  for (int i = 0; i < 10; i++) {
    if (buf1[i] != 0 || buf2[i] != i + 1) {
      asm volatile("cli;hlt;hlt;");
    }
  }
  
  int *channel = reinterpret_cast<int *>(MemoryMap::kF2h);
  if (channel[0] == 0) {
    int16_t id;
    asm volatile("movw %%fs:0x0, %0" : "=r"(id));
    channel[0] = 1 | (id << 16);
  }
  asm volatile("cli;hlt;hlt;");
  return 0;
}
