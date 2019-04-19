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

/*
DOC START

# architecture/trampoline (a.k.a friend16)
The objective of the trampoline code is to switch CPU mode (from real mode to
long mode) and transit from low memory to high memory. This transition is
required because of the limitation at x86 boot sequence: x86 can only boot from
low memory (< 0x100000).

The trampoline code, which size must be under 4KB, is copied from 0x40000000 to
0x70000(TRAMPOLINE_ADDR). The latter page is for the processor boot sequence.
When a friend core is woken up by INIT IPI, it starts execution on the latter
page. After virtual memory is initialized, the trampoline code jumps to the
former page and continues execution.

The address 0x70000 is reserved by the boot parameter of Linux kernel.
(memmap=0x70000\$4K)

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
