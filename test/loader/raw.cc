using uint64_t = __UINT64_TYPE__;
#include "_memory.h"

extern "C" int main() {
  int *channel = reinterpret_cast<int *>(MemoryMap::kF2h);
  channel[0] = 1;
  asm volatile("cli;hlt;hlt;");
  return 0;
}
