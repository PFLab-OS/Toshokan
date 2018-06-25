using uint64_t = __UINT64_TYPE__;
#include "common/_memory.h"
#include "int.h"

Idt idt;

int main() {
  // Setup Interrupt 
  idt.SetupGeneric();
  idt.SetupProc();

  // Load Elf
  int *channel = reinterpret_cast<int *>(MemoryMap::kF2h);
  channel[0] = 6;

}
