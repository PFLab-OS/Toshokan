/*
 * format of trampoline region
 */
// TODO refactoring
enum MemoryMap {
  kMemoryMapJmp0x20                = 0x0,
  kMemoryMapSignature              = 0x4,
  kMemoryMapRegionOffset           = 0x8,
  kMemoryMapTrampolineBinLoadPoint = 0x8,
  kMemoryMapPhysAddrStart          = 0x10,
  kMemoryMapReserved1              = 0x18,
  kMemoryMapTrampolineBinEntry     = 0x20,
  kMemoryMapPml4t                  = 0x1000,
  kMemoryMapPdpt                   = 0x2000,
  kMemoryMapReserved2              = 0x3000,
  kMemoryMapStack                  = 0x4000,
  kMemoryMapH2f                    = 0x5000,
  kMemoryMapF2h                    = 0x6000,
  kMemoryMapEnd                    = 0x7000,
};

int main() {
  int *address = (int *)kMemoryMapF2h;
  address[0] = 1;
  while(1) {
    asm volatile("cli;hlt;nop;");
  };
}
