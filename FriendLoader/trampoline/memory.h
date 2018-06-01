#pragma once

#define PRESENT_BIT  (1<<0)
#define WRITE_BIT    (1<<1)
#define USER_BIT     (1<<2)
#define HUGEPAGE_BIT (1<<7)

#define PML4T        0x1000
#define PDPT         0x2000
#define STACK_BOTTOM 0x5000

#ifndef ASM_FILE

#include "type.h"

enum class MemoryMap : uint64_t {
  kJmp0x20                = 0x0,
  kSignature              = 0x4,
  kRegionOffset           = 0x8,
  kTrampolineBinLoadPoint = 0x8,
  kPhysAddrStart          = 0x10,
  kReserved1              = 0x18,
  kTrampolineBinEntry     = 0x20,
  kPml4t                  = 0x1000,
  kPdpt                   = 0x2000,
  kReserved2              = 0x3000,
  kStack                  = 0x4000,
  kH2f                    = 0x5000,
  kF2h                    = 0x6000,
  kEnd                    = 0x7000,
};

static_assert(static_cast<uint64_t>(MemoryMap::kPml4t) == PML4T, "");
static_assert(static_cast<uint64_t>(MemoryMap::kPdpt)  == PDPT, "");
static_assert(static_cast<uint64_t>(MemoryMap::kStack) + 0x1000 == STACK_BOTTOM, "");


#endif /* ASM_FILE */
