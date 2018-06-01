#pragma once

#include "type.h"

#define kassert(flag)                       \
  if (!(flag)) {                            \
    asm volatile("cli; nop; hlt;");         \
  }

static inline bool disable_interrupt() {
  uint64_t if_flag;
  asm volatile("pushfq; popq %0; andq $0x200, %0;" : "=r"(if_flag));
  bool did_stop_interrupt = (if_flag != 0);
  asm volatile("cli;");
  return did_stop_interrupt;
}

static inline void enable_interrupt(bool flag) {
  if (flag) {
    asm volatile("sti");
  }
}

typedef uint64_t virt_addr;

#define KERNEL_CS (0x10)
#define KERNEL_DS (0x18)
#define USER_DS (0x23)
#define USER_CS (0x2B)

#define MASK(val, ebit, sbit) \
  ((val) & (((1 << ((ebit) - (sbit) + 1)) - 1) << (sbit)))
