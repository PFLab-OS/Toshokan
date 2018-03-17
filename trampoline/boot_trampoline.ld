OUTPUT_FORMAT(binary);

ENTRY (entry)

SECTIONS{
  . = 0x08;
  .text : {
    KEEP(*(.text.boot))
    *(.text)
  }
}
