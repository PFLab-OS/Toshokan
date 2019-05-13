#pragma once

#include "symbol.h"

#include <stddef.h>
#include <stdint.h>
struct Page {
  uint64_t entry[512];
} __attribute__ ((aligned(4096)));

// initialized by hakase.cc (before friend app is executed)
extern SHARED_SYMBOL(int64_t sync_flag);
extern SHARED_SYMBOL(Page pml4t);
extern SHARED_SYMBOL(Page pdpt);
extern SHARED_SYMBOL(Page pd);
