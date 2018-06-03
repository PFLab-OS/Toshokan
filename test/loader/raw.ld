OUTPUT_FORMAT(binary);

ENTRY (main)

SECTIONS{
  .text : {
    KEEP(*(.text.boot))
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
