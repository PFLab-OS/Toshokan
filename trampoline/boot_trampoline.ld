OUTPUT_FORMAT(binary);

ENTRY(entry)

SECTIONS {
  . = 0x40000000;
  .text : AT(0) {
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
