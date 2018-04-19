void panic();

extern "C" void trampoline_main() {
  char *address = (char *)0x1000;
  while(address[0] != 'a') {
    asm volatile("":::"memory");
  }
  panic();
}

void panic() {
  while(true) {
    asm volatile("cli;hlt;nop;");
  }
}
