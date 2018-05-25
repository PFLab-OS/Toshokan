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
  enum class EoiType {
    kNone,
    kLapic,
    kIoapic,
  };
  void SetupGeneric();
  void SetupProc();
  // I/O等用の割り込みハンドラ
  // 確保したvectorが返る(vector >= 64)
  // 確保できなかった場合はReservedIntVector::kErrorが返る
  int SetIntCallback(int_callback callback, void *arg,
                     EoiType eoi);
  // 一括で連続したvectorを確保する
  // rangeは2のn乗である必要がある
  // 戻り値は先頭vectorで、rangeで割り切れる事を保証する
  int SetIntCallback(int_callback *callback, void **arg, int range,
                     EoiType eoi);
  // 例外等用の割り込みハンドラ
  // vector < 64でなければならない
  void SetExceptionCallback(int vector, int_callback callback,
                            void *arg, EoiType eoi);

  volatile int GetHandlingCnt() {
    if (!_is_gen_initialized) {
      return 0;
    }
    return _handling_cnt;
  }
  struct ReservedIntVector {
    static const int kError = -1;
  };

  static const uint16_t kIntVectorNum = 35;
 private:
  void SetGate(idt_callback gate, int vector, uint8_t dpl, bool trap,
               uint8_t ist);
  static void HandlePageFault(Regs *rs, void *arg);
  static void HandleGeneralProtectionFault(Regs *rs, void *arg);
  static const uint32_t kIdtPresent = 1 << 15;
  volatile uint16_t _idtr[5];
  struct IntCallback {
    int_callback callback;
    void *arg;
    EoiType eoi;
  } _callback[kIntVectorNum];
  friend void C::handle_int(Regs *rs);
  int _handling_cnt = 0;
  bool _is_gen_initialized = false;
};
