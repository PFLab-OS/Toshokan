#define ASM_FILE
#include "memory.h"	
	
	.section ".text.boot"
	.code16
	.globl entry
	
	/* written by FriendLoader */
region_addr:
	.quad 0
phys_addr_start:
	.quad 0
reserved:	
	.quad 0
	
entry:
	cli
	movw %cs, %ax
	movw %ax, %ds

	//
	// setup paging
	//
	movl (region_addr), %edx
	leal PML4T(%edx), %edi
	
	// setup PML4T
	leal PDPT(%edx), %eax
	orl  $(PRESENT_BIT | WRITE_BIT | USER_BIT), %eax
	movl %eax, (PML4T)
	movl $0, (PML4T + 4)

	// setup PDPT
	movl (phys_addr_start), %eax
	orl  $(PRESENT_BIT | WRITE_BIT | USER_BIT | HUGEPAGE_BIT), %eax
	movl %eax, (PDPT)
	movl (phys_addr_start + 4), %eax
	movl %eax, (PDPT + 4)

	// load cr3
	movl %edi, %cr3
	
	// set PAE&PGE bit
	movl %cr4, %eax
	orl $(1 << 5 | 1 << 7), %eax
	movl %eax, %cr4

	//
	// switch to long mode
	//
	
	// set LM bit
	movl $0xC0000080, %ecx
	rdmsr
	orl $(1 << 8), %eax
	wrmsr

	// set PG&PE bit
	movl %cr0, %eax
	orl  $(1 << 31 | 1 << 0), %eax
	andl $(~(1 << 30 | 1 << 29)), %eax
	movl %eax, %cr0

	// We enabled paging.
	// CPU will keep running this code, but the physical address is changed.
	// physical address: (phys_addr) + (region_addr) + %rip
	// linear address: (region_addr) + %rip  # as 16bit segmentation is valid

	// setup gdt
	movl (region_addr), %edx
	addl %edx, (gdt_desc_ptr)
	lgdtl gdtr

	// jump to start
	movl $start, (gdtr_ptr)
	ljmpl *(gdtr_ptr)


stop:	
	hlt
	jmp stop

.code64
.extern trampoline_main
start:
	// Now we use gdt_desc (64bit segmentation)
	// physical address: (phys_addr) + %rip
	// linear address: %rip
	  
	// TODO Issue #1
	// relocate page tables and gdt descriptors

  movw $0x18, %ax
	movw %ax, %ds
	movw %ax, %ss
	movq $STACK_BOTTOM, %rsp
	  
	jmp trampoline_main

	.balign 8
gdt_desc:
	// null descriptor
	.quad 0
	.quad 0
	.quad 0x00209a0000000000  // kernel code descriptor
	.quad 0x0000920000000000  // kernel data descriptor
	.quad 0x0020fa0000000000  // user code descriptor
	.quad 0x0000f20000000000  // user data descriptor

	.word 0
gdtr:
	.word 8*6 - 1
gdt_desc_ptr:	
	.quad gdt_desc

gdtr_ptr:
	.long 0
	.long 0x10
