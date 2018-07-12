using int16_t = __INT16_TYPE__;
using uint64_t = __UINT64_TYPE__;
#include "common/_memory.h"
#include "int.h"

Idt idt;

static void HandleTest(Regs *rs, void *arg) {
  int *channel = reinterpret_cast<int *>(MemoryMap::kI2h);
  int16_t id;
  asm volatile("movw %%fs:0x0, %0" : "=r"(id));
  channel[0] = 5 | (id << 16);
  asm volatile("cli;hlt;hlt;");
}

int main() {
  idt.SetupGeneric();
  idt.SetupProc();

  idt.SetExceptionCallback(Idt::ReservedIntVector::kTest, HandleTest, nullptr);

  asm volatile("int $32");
}
