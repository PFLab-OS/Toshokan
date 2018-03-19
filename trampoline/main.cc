void panic();

extern "C" void trampoline_main() {
  panic();
}

void panic() {
  while(true) {
    asm volatile("cli;hlt;nop;");
  }
}
