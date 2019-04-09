OUTPUT_FORMAT(binary);

ENTRY(entry32)

SECTIONS {
  . = 0x40000000;
  .text : AT(0) {
    *(.text.boot)
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
