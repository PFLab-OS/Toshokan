	.text
	.globl int_handler1
	.globl int_handler2
int_handler1:
	iretq

int_handler2:
	movl $1, %ebx                 // ebx = 1
	iretq
