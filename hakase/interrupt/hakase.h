#include "common/channel.h"

struct Regs {
  uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi, r8, r9, r10, r11, r12, r13, r14,
      r15;
  uint64_t n, ecode, rip, cs, rflags;
  uint64_t rsp, ss;
} __attribute__((__packed__));

typedef void (*int_callback)(void *arg);

class InterruptController {
public:
  InterruptController(I2H &i2h) : _i2h(i2h) {
  }
  InterruptController() = delete;

  // Processing Interrupt
  // Return value : true - interrupt handler is registered, false - otherwise
  bool ProcessInterrupt(int64_t &type);

  void Init();

  // Interrupt Handler for I/O
  // Return Value : allocated vector number or ReservedIntVector::kError (if failed)
  int SetIntCallback(int_callback callback, void *arg);

  // Interrupt Handler for Exception
  void SetExceptionCallback(int vector, int_callback callback, void *arg);

  struct ReservedIntVector {
    static const int kError = -1;
  };

  static const int kIntVectorNum = 256;
private:
  I2H &_i2h;
  struct IntCallback {
    int_callback callback;
    void *arg;
  } _callback[kIntVectorNum];
};

