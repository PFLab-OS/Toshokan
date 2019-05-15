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

#define DEPLOY_PHYS_ADDR_START 0x40000000UL
#define DEPLOY_PHYS_ADDR_END 0x80000000UL
#define DEPLOY_PHYS_MEM_SIZE (DEPLOY_PHYS_ADDR_END - DEPLOY_PHYS_ADDR_START)
// restriction of TRAMPOLINE_ADDR from the viewpoint of x86 architecture
//                                     (ref. Multiprocessor Specification)
// - paddr must be <0x100000
// - 0xA0000-0xBF000 is reserved.
#define TRAMPOLINE_ADDR 0x70000
#define TMP_WRAPPER1(x) #x
#define TMP_WRAPPER2(x) TMP_WRAPPER1(x)
#define TRAMPOLINE_ADDR_STR TMP_WRAPPER2(TRAMPOLINE_ADDR)

#define PAGE_SIZE 0x1000
