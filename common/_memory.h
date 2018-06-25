#pragma once

/*
 * format of trampoline region
 */
#ifdef __cplusplus
enum class MemoryMap : uint64_t {
#define DEFINE_MEMMAP(name, value) k##name = value
#else /* __cplusplus */
enum MemoryMap {
#define DEFINE_MEMMAP(name, value) kMemoryMap##name = value
#endif /* __cplusplus */
  DEFINE_MEMMAP(Jmp0x20, 0x0),
  DEFINE_MEMMAP(Signature, 0x4),
  DEFINE_MEMMAP(RegionOffset, 0x8),
  DEFINE_MEMMAP(TrampolineBinLoadPoint, 0x8),
  DEFINE_MEMMAP(PhysAddrStart, 0x10),
  DEFINE_MEMMAP(Id, 0x18),
  DEFINE_MEMMAP(TrampolineBinEntry, 0x20),
  DEFINE_MEMMAP(Pml4t, 0x1000),
  DEFINE_MEMMAP(Pdpt, 0x2000),
  DEFINE_MEMMAP(Pd, 0x3000),
  DEFINE_MEMMAP(Reserved1, 0x4000),
  DEFINE_MEMMAP(H2f, 0x5000),
  DEFINE_MEMMAP(F2h, 0x6000),
  DEFINE_MEMMAP(PerCoreStruct, 0x7000),
  DEFINE_MEMMAP(End, 0x8000),
  DEFINE_MEMMAP(Stack, 0x8000),
};

#undef DEFINE_MEMMAP
