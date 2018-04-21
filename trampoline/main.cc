using int32_t = int;

void panic();

extern "C" void trampoline_main() {
  char *address = (char *)0x1000;
  while(*((int32_t *)address) != 1) {
    asm volatile("":::"memory");
  }
  *((int32_t *)0x2000) = 1;
  panic();
}

void panic() {
  while(true) {
    asm volatile("cli;hlt;nop;");
  }
}
