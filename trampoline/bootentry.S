#define ASM_FILE
#include "memory.h"	
	
	.section ".text.boot"
	.code16
	.globl entry
	
	/* written by FriendLoader */
phys_addr_start:
	.quad 0
gdt_desc32:
	.quad 0
	.quad 0
	.quad 0
reserved:
	.quad 0
id:
	.quad 0
stack_virt_addr:
	.quad 0	

entry:
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
	ljmpl $0x08, $entry32
	
	.balign 8
	.word 0
gdtr32:
	.word 8*4 - 1
	.long gdt_desc32 + TRAMPOLINE_ADDR

.code32
entry32:
	movw $0x10, %ax
	movw %ax, %ds
	
	//
	// setup paging
	// page tables are initialized by FriendLoader/trampoline_loader.c
	//
	movl (phys_addr_start), %edx
	leal PML4T(%edx), %edi
	
	// load cr3
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
	lgdtl gdtr

	// jump to start
	ljmpl $0x10, $start


stop:	
	hlt
	jmp stop

.code64
.extern trampoline_main
start:
	// Now we use gdt_desc (64bit segmentation)
	// physical address: (phys_addr) + %rip
	// linear address: %rip

	movw $0x18, %ax
	movw %ax, %ds
	movw %ax, %ss
	movq (stack_virt_addr), %rax
	movq %rax, %rsp
	xorq %rax, %rax
	jmp trampoline_main

	.balign 8
gdt_desc:	
	.quad 0                   // null descriptor
	.quad 0                   // null descriptor
	.quad 0x00209a0000000000  // kernel code descriptor
	.quad 0x0000920000000000  // kernel data descriptor
	.quad 0x0020fa0000000000  // user code descriptor
	.quad 0x0000f20000000000  // user data descriptor

	.word 0
gdtr:
	.word 8*6 - 1
	.quad gdt_desc
