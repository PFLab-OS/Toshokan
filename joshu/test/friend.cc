using uint64_t = __UINT64_TYPE__;
#include "common/_memory.h"

int main() {

  int *channel = reinterpret_cast<int *>(MemoryMap::kF2h);
  channel[0] = 7;

  return 0;
}
