ENTRY(_start)

SECTIONS
{
	. = 0x0; /* 0x40000000; */
	.startup . : { startup.o(.text) }
	.text : { *(.text) }
	.data : { *(.data) }
	.bss : { *(.bss COMMON) }
	. = ALIGN(8);
	. = . + 0x10000; /* 64kB of stack memory */
	stack_top = .;
}
