#define ASM_FILE
#include <toshokan/memory.h>

/*
DOC START

# architecture/trampoline (a.k.a friend16)
The objective of the trampoline code is to switch CPU mode (from real mode to
long mode) and transit from low memory to high memory. This transition is
required because of the limitation at x86 boot sequence: x86 can only boot from
low memory (< 0x100000).

The address 0x70000 is reserved by the boot parameter of Linux kernel.
(memmap=0x70000\$8K)

DOC END
*/
	.text
	.code16
	.globl entry16
	
entry16:
	jmp _entry16

	.balign 8
addr:	
	.quad DEPLOY_PHYS_ADDR_START   // overwritten by Loader16::Init()

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

	.balign 8
	.word 0
gdtr32:
	.word 8*4 - 1
	.long gdt_desc32 + TRAMPOLINE_ADDR

gdt_desc32:
	.quad 0                    // null
	.quad 0x00CF9A000000FFFFUL // kernel code
	.quad 0x00CF92000000FFFFUL // kernel data

	.code32
entry32:
	movw $0x10, %ax
	movw %ax, %ds

	// load cr3
	movl $INITIAL_PML4T, %edi
	movl %edi, %cr3
	
	// set PAE&PGE&PSE bit
	movl %cr4, %eax
	orl $((1 << 4) | (1 << 5) | (1 << 7)), %eax
	movl %eax, %cr4

	//
	// switch to long mode
	//
	
	// set LM bit
	movl $0xC0000080, %ecx
	rdmsr
	orl $(1 << 8), %eax
	wrmsr

	// set PG bit
	movl %cr0, %eax
	orl  $(1 << 31), %eax
	andl $(~((1 << 30) | (1 << 29))), %eax // ensure cache enable & write back
	movl %eax, %cr0

	// We enabled paging.
	
	// setup gdt
	lgdtl TRAMPOLINE_ADDR + gdtr64

	ljmpl $0x10, $(TRAMPOLINE_ADDR + start)

	.code64
start:
	// Now we use gdt_desc (64bit segmentation)
	// physical address: (phys_addr) + %rip
	// linear address: %rip
	movw $0x18, %ax
	movw %ax, %ds
	movw %ax, %ss

	movq $(TRAMPOLINE_ADDR + addr), %rax
	jmpq *(%rax)
	
	.balign 8
gdt_desc64:	
	.quad 0                   // null descriptor
	.quad 0                   // null descriptor
	.quad 0x00209a0000000000  // kernel code descriptor
	.quad 0x0000920000000000  // kernel data descriptor
	.quad 0x0000f20000000000  // user data descriptor
	.quad 0x0020fa0000000000  // user code descriptor

	.word 0
gdtr64:
	.word 8*6 - 1
	.quad TRAMPOLINE_ADDR + gdt_desc64

