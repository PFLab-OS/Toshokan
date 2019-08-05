	.text
	.globl int_handler
int_handler:
	hlt
	jmp int_handler
