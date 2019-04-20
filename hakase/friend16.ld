OUTPUT_FORMAT(binary);

SECTIONS {
  . = 0x0;
  .text : {
    *(.text)
  }
}
