	.text
	.globl int_handler1
	.globl int_handler2
	.extern zero
int_handler1:
	iretq

int_handler2:
	movl $1, (zero)                 // zero = 1
	iretq
