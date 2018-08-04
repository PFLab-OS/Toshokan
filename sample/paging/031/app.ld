OUTPUT_FORMAT(binary);

SECTIONS{
  . = 0xC0000000;
  .text : {
    KEEP(*(.text.entry))
    *(.text)
    *(.gnu.linkonce.t*)
  }
  .rodata : {
    *(.rodata*)
    *(.gnu.linkonce.r*)
  }
  .data : {
    *(.data*)
    *(.gnu.linkonce.d*)
  }
  .bss : {
    *(.bss*)
    *(.gnu.linkonce.b*)
  }
}
