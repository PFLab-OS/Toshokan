#define ASM_FILE
#include <toshokan/symbol.h>

	.text
	.code64
	.globl entry64
	.extern SHARED_SYMBOL(sync_flag)
entry64:
	movabsq $SHARED_SYMBOL(sync_flag), %rax
	lock addq $1, (%rax)

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
