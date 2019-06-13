#pragma once

#include <toshokan/offload.h>
#include <toshokan/symbol.h>

#include <stddef.h>
#include <stdint.h>
struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

// initialized by hakase.cc (before friend app is executed)
extern int64_t SHARED_SYMBOL(sync_flag);
extern Page SHARED_SYMBOL(pml4t);
extern Page SHARED_SYMBOL(pdpt);
extern Page SHARED_SYMBOL(pd);
extern Offloader SHARED_SYMBOL(offloader);
