#define ASM_FILE
#include "memory.h"
	
	.text
	.code16
	.globl entry16
	
entry16:
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
	ljmpl $0x08, $DEPLOY_PHYS_ADDR_START
	
	.balign 8
	.word 0
gdtr32:
	.word 8*4 - 1
	.long gdt_desc32 + TRAMPOLINE_ADDR

gdt_desc32:
	.quad 0                    // null
	.quad 0x00CF9A000000FFFFUL // kernel code
	.quad 0x00CF92000000FFFFUL // kernel data
