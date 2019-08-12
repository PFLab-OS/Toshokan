	.text
	.globl int_handler1
	.globl int_handler2
	.extern pt_entry
int_handler1:
	add  $8, %rsp // to remove error code
	iretq

int_handler2:
	pushq %rax
	movq (pt_entry), %rax
	movq $(0x40000000 | (1 << 0) | (1 << 1) | (1 << 2)), (%rax)
	movq $0x80000000, %rax
	invlpg (%rax)
	popq %rax
	add  $8, %rsp // to remove error code
	iretq
