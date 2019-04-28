#define ASM_FILE
#include "memory.h"

/*
DOC START

# architecture/trampoline (a.k.a friend16)
The objective of the trampoline code is to switch CPU mode (from real mode to
protect mode) and transit from low memory to high memory. This transition is
required because of the limitation at x86 boot sequence: x86 can only boot from
low memory (< 0x100000).

The address 0x70000 is reserved by the boot parameter of Linux kernel.
(memmap=0x70000\$4K)

DOC END
*/
	.text
	.code16
	.globl entry16
	
entry16:
	jmp _entry16

	.balign 4
addr:	
	.long DEPLOY_PHYS_ADDR_START   // overwritten by Loader16::Init()

_entry16:	
	cli
	movw %cs, %ax
	movw %ax, %ds

	//
	// switch to protect mode
	//
	lgdtl gdtr32

	// set PE bit
	movl %cr0, %eax
	orl $(1 << 0), %eax
	movl %eax, %cr0
	ljmpl $0x08, $(TRAMPOLINE_ADDR + entry32)

	.code32
entry32:
	movw $0x10, %ax
	movw %ax, %ds
	
	jmpl *(TRAMPOLINE_ADDR + addr)
	
	.balign 8
	.word 0
gdtr32:
	.word 8*4 - 1
	.long gdt_desc32 + TRAMPOLINE_ADDR

gdt_desc32:
	.quad 0                    // null
	.quad 0x00CF9A000000FFFFUL // kernel code
	.quad 0x00CF92000000FFFFUL // kernel data
