#define ASM_FILE

	.section ".text.boot"
	.code32
	.globl entry32
	.extern pml4t
entry32:
	// load cr3
	movl $pml4t, %edi
	movl %edi, %cr3
	
	// set PAE&PGE&PSE bit
	movl %cr4, %eax
	orl $((1 << 4) | (1 << 5) | (1 << 7)), %eax
	movl %eax, %cr4

	//
	// switch to long mode
	//
	
	// set LM bit
	movl $0xC0000080, %ecx
	rdmsr
	orl $(1 << 8), %eax
	wrmsr

	// set PG bit
	movl %cr0, %eax
	orl  $(1 << 31), %eax
	andl $(~((1 << 30) | (1 << 29))), %eax // ensure cache enable & write back
	movl %eax, %cr0

	// We enabled paging.
	
	// setup gdt
	lgdtl gdtr

	// jump to start
	ljmpl $0x10, $start


stop:	
	hlt
	jmp stop

.text
.code64
.extern sync_flag
start:
	// Now we use gdt_desc (64bit segmentation)
	// physical address: (phys_addr) + %rip
	// linear address: %rip

	movw $0x18, %ax
	movw %ax, %ds
	movw %ax, %ss
	lock addq $1, sync_flag
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
