#include"type.h"
#include "channel.h"

void panic();

// callback test
void callback(H2F &h2f, F2H &f2h) {
  h2f.Return(0);
  f2h.SendSignal(1);
}

// print request test
void print(H2F &h2f, F2H &f2h) {
  h2f.Return(0);
  f2h.WriteString("abc\n");
}

// execute binary
void exec_bin(H2F &h2f, F2H &f2h) { h2f.Return(-1); }

void rw_memory(H2F &h2f, F2H &f2h) {
  static const uint32_t kRead = 0;
  static const uint32_t kWrite = 1;
  
  uint32_t type;
  uint64_t address_;
  uint64_t *buf = h2f.GetRawPtr<uint64_t>() + 2048 / sizeof(uint64_t);
  
  h2f.Read(0, type);
  h2f.Read(8, address_);

  uint64_t *address = reinterpret_cast<uint64_t *>(address_);
  if (type == kRead) {
    for(int i = 0; i < 2048 / sizeof(uint64_t); i++) {
      buf[i] = address[i];
    }
  } else if (type == kWrite) {
    for(int i = 0; i < 2048 / sizeof(uint64_t); i++) {
      address[i] = buf[i];
    }
  } else {
    h2f.Return(-1);
    return;
  }
  
  h2f.Return(0);
}

void do_signal(H2F &h2f) {
}

extern "C" void trampoline_main() {
  H2F h2f;
  F2H f2h;
  while(true) {
    switch (h2f.WaitNewSignal()) {
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
    case 4:
      rw_memory(h2f, f2h);
      break;
    }
  }
}

void panic() {
  while (true) {
    asm volatile("cli;hlt;nop;");
  }
}
