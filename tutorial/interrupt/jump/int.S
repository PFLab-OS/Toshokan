	.text
	.globl int_handler1
	.globl int_handler2
	.globl int_handler3
	.globl int_handler4
	.extern interrupt_count
	.extern zero
int_handler1:
int_handler2:
	hlt
	jmp int_handler2

int_handler3:
	incl (interrupt_count)		// interrupt_count++;
	cmpl $2, (interrupt_count)	// 2 == interrupt_count
	je stop_int_handler3            // if it's true, jump to stop_int_handler2
	iretq				// return from the interrupt handler
	
stop_int_handler3:
	hlt
	jmp stop_int_handler3

int_handler4:
	movl $1, (zero)                 // zero = 1
	iretq
