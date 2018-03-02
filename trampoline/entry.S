	.section ".text.boot"
	.code16
	.globl entry

	/* written by FriendLoader */
region_addr:
	.quad 0
phys_addr_start:
	.quad 0
phys_addr_end:	
	.quad 0
	
entry:
	cli
	hlt
	hlt
	hlt
	nop
