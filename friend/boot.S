#define ASM_FILE
#define STACK_SIZE 0x1000

	.text
	.code64
	.globl entry64
	.extern friend_init
	.extern friend_binary_end
entry64:
	// align friend_binary_end to 4096
	//
	//  extern char *friend_binary_end;
	//  rbx = (((uint64_t)friend_binary_end) / 4096) * 4096;
	movabsq $friend_binary_end, %rbx
	sarq $12, %rbx
        salq $12, %rbx

	// calculate stack address atomically
	//
	//  eax = __sync_fetch_and_add(&cnt_flag, STACK_SIZE);
	//  rax = (uint64_t)eax;
	//  rax += rbx;
	//  rsp = rax;
	movl $STACK_SIZE, %eax
	movabsq $cnt_flag, %rbx
	lock xaddl %eax, (%rbx)
	cltq
	addq %rbx, %rax
	movq %rax, %rsp
	movabsq $friend_init, %rbx
	call *%rbx

stop:	
	pause
	jmp stop

cnt_flag:
	.long 0x2000
	
