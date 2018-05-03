#include "type.h"
#include "channel.h"

void panic();

// callback test
void callback(H2F &h2f, F2H &f2h) {
  h2f.SetType(0);
  f2h.SetType(1);
}

// print request test
void print(H2F &h2f, F2H &f2h) {
  h2f.SetType(0);
  f2h.WriteString("abc\n");
}

// execute binary
void exec_bin(H2F &h2f, F2H &f2h) { h2f.SetType(-1); }

void do_signal(H2F &h2f) {
  F2H f2h;
  switch (h2f.GetType()) {
  case 0:
    panic();
  case 1:
    callback(h2f, f2h);
    break;
  case 2:
    print(h2f, f2h);
    break;
  case 3:
    exec_bin(h2f, f2h);
    break;
  }
}

extern "C" void trampoline_main() {
  H2F h2f;
  h2f.WaitNewSignal();
  do_signal(h2f);
  panic();
}

void panic() {
  while (true) {
    asm volatile("cli;hlt;nop;");
  }
}
