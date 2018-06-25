using uint64_t = __UINT64_TYPE__;
#include "common/_memory.h"

int main() __attribute__ ((section (".text.boot")));

int main() {
  unsigned int *channel = reinterpret_cast<unsigned int *>(MemoryMap::kF2h);
  channel[0] = 1 | (1 << 16); // TODO replace to ID
  asm volatile("cli;hlt;hlt;");
  return 0;
}
