#pragma once

#define PRESENT_BIT (1 << 0)
#define WRITE_BIT (1 << 1)
#define USER_BIT (1 << 2)
#define HUGEPAGE_BIT (1 << 7)

#define PML4T 0x1000
#define PDPT 0x2000
#define PD 0x3000
#define TMP_STACK 0x9000

#include "common/_memory.h"

#ifndef ASM_FILE

#include "type.h"

static_assert(static_cast<uint64_t>(MemoryMap::kPml4t) == PML4T, "");
static_assert(static_cast<uint64_t>(MemoryMap::kPdpt) == PDPT, "");
static_assert(static_cast<uint64_t>(MemoryMap::kPd) == PD, "");
static_assert(static_cast<uint64_t>(MemoryMap::kTemporaryStack) + 0x1000 == TMP_STACK, "");

#endif /* ASM_FILE */
