	.text
	.globl int_handler
	.extern fault_handler
int_handler:
	pushq %r15
	pushq %r14
	pushq %r13
	pushq %r12
	pushq %r11
	pushq %r10
	  pushq %r9
	  pushq %r8
	  pushq %rdi
	  pushq %rsi
	  pushq %rbp
	  pushq %rdx
	  pushq %rcx
	  pushq %rbx
	  pushq %rax
	  call fault_handler
	  popq %rax
	  popq %rbx
	  popq %rcx
	  popq %rdx
	  popq %rbp
	  popq %rsi
	  popq %rdi
	  popq %r8
	  popq %r9
	  popq %r10
	  popq %r11
	  popq %r12
	  popq %r13
	  popq %r14
	  popq %r15
	add  $8, %rsp // to remove error code
	iretq
