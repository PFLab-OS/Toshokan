OUTPUT_FORMAT(elf64-x86-64)

ENTRY (main)

SECTIONS{
  . = SEGMENT_START("text-segment", 0x200000) + SIZEOF_HEADERS;
  .text : {
    *(.text*)
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
    *(COMMON*)
    *(.COMMON*)
    *(.bss*)
    *(.gnu.linkonce.b*)
  }

  .note : {
    *(.note*)
    *(.eh_frame .eh_frame_hdr)
    *(.comment*)
  }

  .unsupported : {
    *(*)
    ASSERT(0, "Found non-empty unsupported section in object files.");
  }
}
