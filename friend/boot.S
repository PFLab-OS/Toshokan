#define ASM_FILE
#define STACK_SIZE 0x4000

	.text
	.code64
	.globl entry64
	.extern friend_init
	.extern friend_binary_end
entry64:
	movabsq $friend_binary_end, %rcx

	// calculate stack address atomically
	//
	//  eax = __sync_fetch_and_add(&stack_offset, STACK_SIZE);
	//  rax = (uint64_t)eax;
	//  rax += rcx;
	//  rsp = rax;
	movl $STACK_SIZE, %eax
	movabsq $stack_offset, %rbx
	lock xaddl %eax, (%rbx)
	cltq
	addq %rcx, %rax
	movq %rax, %rsp
	movabsq $friend_init, %rbx
	call *%rbx

stop:	
	pause
	jmp stop

stack_offset:
	.long 0x2000
	
