#pragma once

/*
DOC START

# architecture/memory
## physical memory
0GB-1GB for hakase & Linux
1GB-2GB for friend (reserved by Linux kernel boot parameter, Linux does not use
this region)

At the top of the friend region, there is a trampoline header which includes
information passed from FriendLoader. You can see the the detail of it at enum
MemoryMap.

## virtual memory
The virtual memory of friend OS is initialized by trampoline.
A Friend OS can set its own virtual memory space. When you try to initialize
memory space from scratch, keep attention not to unmap some important
pages.(e.g. channels, the trampoline header)

DOC END
*/

#ifndef ASM_FILE

#ifdef __FRIEND_LOADER__
#include <linux/types.h>
#else
#include <stdint.h>
#endif /* __FRIEND_LOADER__ */

#ifdef __cplusplus

enum class MemoryMap : uint64_t {
#define DEFINE_MEMMAP(name, value) k##name = value

#else /* __cplusplus */

enum MemoryMap {
#define DEFINE_MEMMAP(name, value) kMemoryMap##name = value

#endif /* __cplusplus */
  DEFINE_MEMMAP(Jmp0x30, 0x0),
  DEFINE_MEMMAP(Reserved1, 0x4),
  DEFINE_MEMMAP(TrampolineBinLoadPoint, 0x8),
  DEFINE_MEMMAP(PhysAddrStart, 0x8),
  DEFINE_MEMMAP(GdtPtr32, 0x10),
  DEFINE_MEMMAP(Reserved2, 0x28),
  DEFINE_MEMMAP(Id, 0x30),
  DEFINE_MEMMAP(StackVirtAddr, 0x38),
  DEFINE_MEMMAP(TrampolineBinEntry, 0x40),
  DEFINE_MEMMAP(Pml4t, 0x1000),
  DEFINE_MEMMAP(Pdpt, 0x2000),
  DEFINE_MEMMAP(Pd, 0x3000),
  DEFINE_MEMMAP(TmpPd, 0x4000),
  DEFINE_MEMMAP(H2f, 0x5000),
  DEFINE_MEMMAP(F2h, 0x6000),
  DEFINE_MEMMAP(I2h, 0x7000),
  DEFINE_MEMMAP(TemporaryStack, 0x8000),
  DEFINE_MEMMAP(PerCoreStruct, 0x9000),
  DEFINE_MEMMAP(End, 0x10000),
  DEFINE_MEMMAP(Stack, 0x100000),
};

static const int kStackSize = 0x100000;
#undef DEFINE_MEMMAP
#endif /* ASM_FILE */

// File deployment
// XXX: mem=1GでLinuxが起動していると仮定
#define DEPLOY_PHYS_ADDR_START 0x40000000UL
#define DEPLOY_PHYS_ADDR_END 0x80000000UL
#define DEPLOY_PHYS_MEM_SIZE (DEPLOY_PHYS_ADDR_END - DEPLOY_PHYS_ADDR_START)
#define TRAMPOLINE_ADDR 0x70000
