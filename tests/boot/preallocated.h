#pragma once

#include <toshokan/memory.h>

// Up to DEPLOY_PHYS_ADDR_START + 0x1000 is pre-allocated area.
// This restricts binary size smaller than 0x1000.

#define SYNCFLAG_ADDR (DEPLOY_PHYS_ADDR_START + 0x1000)
#define PML4T_ADDR (DEPLOY_PHYS_ADDR_START + 0x2000)

#ifndef ASM_FILE
#include <stddef.h>
#include <stdint.h>
struct Page {
  uint64_t entry[512];
};

struct PreallocatedMemory {
  int64_t sync_flag;
  int64_t unused[511];  // for page alignment
  Page pml4t;
  Page pdpt;
  Page pd;
};

static PreallocatedMemory *const preallocated_mem =
    reinterpret_cast<PreallocatedMemory *>(DEPLOY_PHYS_ADDR_START + 0x1000);
#endif
