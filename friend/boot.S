#define ASM_FILE
#include <toshokan/symbol.h>
#define STACK_SIZE 0x1000

	.section ".text.boot"
	.code32
	.globl entry32
	.extern SHARED_SYMBOL(__toshokan_pml4t)
entry32:
	// load cr3
	movl $SHARED_SYMBOL(__toshokan_pml4t), %edi
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

.text
.code64
.extern friend_init
.extern friend_binary_end
start:
	// Now we use gdt_desc (64bit segmentation)
	// physical address: (phys_addr) + %rip
	// linear address: %rip

	movw $0x18, %ax
	movw %ax, %ds
	movw %ax, %ss

	// align friend_binary_end to 4096
	//
	//  extern char *friend_binary_end;
	//  rbx = (((uint64_t)friend_binary_end) / 4096) * 4096;
	movq $friend_binary_end, %rbx
	sarq $12, %rbx
        salq $12, %rbx

	// calculate stack address atomically
	//
	//  eax = __sync_fetch_and_add(&cnt_flag, STACK_SIZE);
	//  rax = (uint64_t)eax;
	//  rax += rbx;
	//  rsp = rax;
	movl $STACK_SIZE, %eax
	lock xaddl %eax, cnt_flag
	cltq
	addq %rbx, %rax
	movq %rax, %rsp
	call friend_init
	
stop:	
	hlt
	jmp stop

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

cnt_flag:
	.long 0x2000
	
