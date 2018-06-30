using uint64_t = __UINT64_TYPE__;
using int16_t = __INT16_TYPE__;
#include "common/_memory.h"

int main() {
  asm volatile("int $32");

  asm volatile("hlt;hlt;");
  return 0;
}
