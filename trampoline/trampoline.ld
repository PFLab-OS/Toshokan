OUTPUT_FORMAT(binary);

ENTRY (entry)

SECTIONS{
  .text : {
    KEEP(*(.text.boot))
    *(.text)
  }
}
