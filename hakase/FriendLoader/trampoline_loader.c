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
When FriendLoader initiates a friend core, it sets a trampoline code for the core.
The objective of the trampoline code is to initialize processor core states, and waits messages from hakase through channels.

The trampoline code is copied at 0x70000(TRAMPOLINE_ADDR)-0x70000+4KB page and at 1GB-1GB+4KB page.
The former page is used by the processor boot sequence. When a friend core is woken up by INIT IPI, it starts its execution on
the former page. After virtual memory initialization (at trampoline/bootentry.S), the trampoline code jumps to the latter page 
and continues its execution.

The address 0x70000 is reserved by the boot parameter of Linux kernel. (memmap=0x70000\$4K)

DOC END
*/

static uint8_t jmp_bin[] = {0xeb, kMemoryMapTrampolineBinEntry - 2, 0x66, 0x90}; // jmp TrampolineBinEntry; xchg %ax, &ax

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

static uint64_t add_base_addr_to_segment_descriptor(uint64_t desc) {
  return desc | ((DEPLOY_PHYS_ADDR_START & 0xFFFFFF) << 16) | ((DEPLOY_PHYS_ADDR_START >> 24) << 56);
}

int trampoline_region_init(struct trampoline_region *region,
                           phys_addr_t phys_addr_start,
                           phys_addr_t phys_addr_end) {
  extern uint8_t _binary_boot_trampoline_bin_start[];
  extern uint8_t _binary_boot_trampoline_bin_end[];
  extern uint8_t _binary_boot_trampoline_bin_size[];
  size_t binary_boot_trampoline_bin_size =
      (size_t)_binary_boot_trampoline_bin_size;
  uint8_t *buf;
  const size_t kRegionSize = binary_boot_trampoline_bin_size + kMemoryMapTrampolineBinLoadPoint;

  {
    uint8_t __iomem *vaddr = ioremap(region->paddr, 0x1000);
    uint64_t __iomem *vaddr64 = (uint64_t __iomem *)vaddr;

    memcpy(vaddr, jmp_bin, sizeof(jmp_bin) / sizeof(jmp_bin[0]));

    if (0x1000 - kMemoryMapTrampolineBinLoadPoint < binary_boot_trampoline_bin_size) {
      return -1;
    }

    if (_binary_boot_trampoline_bin_start + binary_boot_trampoline_bin_size !=
        _binary_boot_trampoline_bin_end) {
      // invalid state
      return -1;
    }

    // copy trampoline binary to trampoline region + 8 byte
    memcpy(vaddr + kMemoryMapTrampolineBinLoadPoint / (sizeof(*vaddr)),
           _binary_boot_trampoline_bin_start, binary_boot_trampoline_bin_size);

    // check address
    if ((phys_addr_start & (2 * 1024 * 1024 - 1)) != 0) {
      // should be aligned to 2MB boundary
      // because of using 2MB huge page
      return -1;
    }

    // initialize trampoline header
    vaddr64[kMemoryMapPhysAddrStart / sizeof(*vaddr64)] = phys_addr_start;
    // null descriptor
    vaddr64[kMemoryMapGdtPtr32 / sizeof(*vaddr64) + 0] = 0;
    // kernel code descriptor
    vaddr64[kMemoryMapGdtPtr32 / sizeof(*vaddr64) + 1] = add_base_addr_to_segment_descriptor(0x00CF9A000000FFFFUL);
    // kernel data descriptor
    vaddr64[kMemoryMapGdtPtr32 / sizeof(*vaddr64) + 2] = add_base_addr_to_segment_descriptor(0x00CF92000000FFFFUL);
    vaddr64[kMemoryMapId / sizeof(*vaddr64)] = 0;            // will be initialized by trampoline_region_set_id()
    vaddr64[kMemoryMapStackVirtAddr / sizeof(*vaddr64)] = 0; // will be initialized by trampoline_region_set_id()

    buf = (uint8_t *)kmalloc(kRegionSize, GFP_KERNEL);
    memcpy_fromio(buf, vaddr, kRegionSize);
    
    iounmap(vaddr);
  }

  if (deploy((const char *)buf, kRegionSize, 0) < 0) {
    pr_err("friend_loader: deploy failed\n");
    return -1;
  }

  if (deploy_zero(kMemoryMapStack, kStackSize * get_cpu_num()) < 0) {
    pr_err("friend_loader: deploy failed\n");
    return -1;
  }

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
