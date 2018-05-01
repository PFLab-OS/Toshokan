#pragma once

#include "type.h"

#define kassert(flag)                       \
  if (!(flag)) {                            \
    asm volatile("cli; nop; hlt;");         \
  }

typedef uint64_t virt_addr;

#define KERNEL_CS (0x10)
#define KERNEL_DS (0x18)
#define USER_DS (0x23)
#define USER_CS (0x2B)

#define MASK(val, ebit, sbit) \
  ((val) & (((1 << ((ebit) - (sbit) + 1)) - 1) << (sbit)))
