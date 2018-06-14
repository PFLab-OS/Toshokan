using uint64_t = __UINT64_TYPE__;
#include "common/_memory.h"
#include "int.h"

Idt idt;

static void HandleTest(Regs *rs, void *arg) {
  int *channel = reinterpret_cast<int *>(MemoryMap::kF2h);
  channel[0] = 5;
  asm volatile("cli;hlt;hlt;");
}

int main() {
  idt.SetupGeneric();
  idt.SetupProc();

  idt.SetExceptionCallback(Idt::ReservedIntVector::kTest, HandleTest, nullptr);

  asm volatile("int $32");
}
