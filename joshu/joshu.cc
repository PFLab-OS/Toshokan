#include "int.h"
#include "channel.h"

Idt idt;
F2H f2h;

static void HandleTest(Regs *rs, void *arg) {

}

int main() {
  // Setup Interrupt 
  idt.SetupGeneric();
  idt.SetupProc();

  idt.SetExceptionCallback(Idt::ReservedIntVector::kTest, HandleTest, nullptr);

  // Load Elf
  f2h.SendSignal(6);

}
