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
    *(.init_array)
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

  .debug_info    0 : { *(.debug_info) }
  .debug_abbrev  0 : { *(.debug_abbrev) }
  .debug_aranges 0 : { *(.debug_aranges) }
  .debug_ranges  0 : { *(.debug_ranges) }
  .debug_line    0 : { *(.debug_line) }
  .debug_str     0 : { *(.debug_str) }

  .unsupported : {
    *(*)
    ASSERT(0, "Found non-empty unsupported section in object files.");
  }
}
