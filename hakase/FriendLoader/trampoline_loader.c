#include "trampoline_loader.h"
#include "deploy.h"
#include "_memory.h"
#include "cpu_hotplug.h"
#include <asm/realmode.h>
#include <linux/slab.h>
#include <linux/gfp.h>

/*
DOC START
 
# architecture/trampoline
The objective of the trampoline code is to switch CPU mode (from real mode to long mode) and transit from low memory to high memory. This transition is required because of the limitation at x86 boot sequence: x86 can only boot from low memory (< 0x100000).

The trampoline code, which size must be under 4KB, is copied from 0x40000000 to 0x70000(TRAMPOLINE_ADDR).
The latter page is for the processor boot sequence. When a friend core is woken up by INIT IPI, it starts execution on
the latter page. After virtual memory is initialized, the trampoline code jumps to the former page 
and continues execution.

The address 0x70000 is reserved by the boot parameter of Linux kernel. (memmap=0x70000\$4K)

DOC END
*/

int trampoline_region_alloc(struct trampoline_region *region) {
  // restriction of TRAMPOLINE_ADDR from the viewpoint of x86 architecture
  //                                     (ref. Multiprocessor Specification)
  // - paddr must be <0x100000
  // - 0xA0000-0xBF000 is reserved.
  region->paddr = TRAMPOLINE_ADDR;

  return 0;
}

void trampoline_region_free(struct trampoline_region *region) {
  region->paddr = 0;
}

int trampoline_region_init(struct trampoline_region *region,
                           phys_addr_t phys_addr_start,
                           phys_addr_t phys_addr_end) {
  return 0;
}

int trampoline_region_set_id(struct trampoline_region *region, int cpuid, int apicid) {
  int32_t buf[2];
  uint64_t stack_addr;

  if (cpuid <= 0) {
    // cpuid 0 must be reserved for hakase
    return -1;
  }

  stack_addr = cpuid * kStackSize + kMemoryMapStack;
  
  buf[0] = apicid;
  buf[1] = cpuid;
  
  if (deploy((const char *)buf, sizeof(buf), kMemoryMapId) < 0) {
    return -1;
  }

  if (deploy((const char *)&stack_addr, sizeof(stack_addr), kMemoryMapStackVirtAddr) < 0) {
    return -1;
  }
  return 0;
}
