	.text
	.globl int_handler1
	.globl int_handler2
	.extern pt_entry
int_handler1:
	add  $8, %rsp // to remove error code
	iretq

int_handler2:
	pushq %rax
	pushq %rbx
	movabsq $pt_entry, %rax
	movq (%rax), %rax  // store page table entry address to rax
	movabsq $(0x100000000UL | (1 << 0) | (1 << 1) | (1 << 2)), %rbx // store page table entry to rbx
	movq %rbx, (%rax) // set page table entry
	movq $0x80000000, %rax
	invlpg (%rax) // clear TLB entry
	popq %rbx
	popq %rax
	add  $8, %rsp // to remove error code
	iretq
