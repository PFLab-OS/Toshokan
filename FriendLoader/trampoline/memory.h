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

#include "common/_memory.h"

static_assert(static_cast<uint64_t>(MemoryMap::kPml4t) == PML4T, "");
static_assert(static_cast<uint64_t>(MemoryMap::kPdpt)  == PDPT, "");
static_assert(static_cast<uint64_t>(MemoryMap::kStack) + 0x1000 == STACK_BOTTOM, "");


#endif /* ASM_FILE */
