#pragma once

#include "trampoline.h"

struct Regs {
  uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14,
      r15;
  uint64_t n, ecode, rip, cs, rflags;
  uint64_t rsp, ss;
} __attribute__((__packed__));

typedef void (*idt_callback)(Regs *rs);
typedef void (*int_callback)(Regs *rs, void *arg);

namespace C {
extern "C" void handle_int(Regs *rs);
}

class Idt {
 public:
  void SetupGeneric();
  void SetupProc();

  // Interrupt Handler for I/O
  // Return Value : allocated vector number or ReservedIntVector::kError (if failed)
  int SetIntCallback(int_callback callback, void *arg);

  // Interrupt Handler for Exception
  void SetExceptionCallback(int vector, int_callback callback, void *arg);

  volatile int GetHandlingCnt() {
    if (!_is_gen_initialized) {
      return 0;
    }
    return _handling_cnt;
  }
  struct ReservedIntVector {
    static const int kError = -1;
    static const int kTest = 32;
  };

  static const uint16_t kIntVectorNum = 33;
 private:
  void SetGate(idt_callback gate, int vector, uint8_t dpl, bool trap, uint8_t ist);
  static const uint32_t kIdtPresent = 1 << 15;
  volatile uint16_t _idtr[5];
  struct IntCallback {
    int_callback callback;
    void *arg;
  } _callback[kIntVectorNum];
  friend void C::handle_int(Regs *rs);
  int _handling_cnt = 0;
  bool _is_gen_initialized = false;
};
