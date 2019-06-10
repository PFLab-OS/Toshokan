/*
DOC START

# friend/binary
## limitations
- It must be placed into friend region.(0x40000000 - 0x80000000)
- It has to switch the mode from protected mode to long mode by itself

# friend/elf_binary
## limitations
- It shares the limitations with friend binaries. (see above)
- Some compile flags must not be used for full features.

## linker script
friend/friend.ld can be used for linker script.
It specifies entry symbol(entry32) and start address(0x40000000).

## compile flags
To resolve symbols between hakase binary and friend binary at compile time,
you should not omit symbol flags of friend binary by '-s(--strip-all)'.

If you want, you have to manage communication between hakase and friend by yourself.

DOC END
*/

OUTPUT_FORMAT(elf64-x86-64)

ENTRY (entry32)

SECTIONS{
  . = SEGMENT_START("text-segment", 0x40000000) + SIZEOF_HEADERS;
  .text : {
    *(.text*)
    *(.gnu.linkonce.t*)
  }

  .rodata : ALIGN(0x1000) {
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
  . = ALIGN(0x1000);
  friend_binary_end = .;

  .debug_info    0 : { *(.debug_info) }
  .debug_abbrev  0 : { *(.debug_abbrev) }
  .debug_aranges 0 : { *(.debug_aranges) }
  .debug_ranges  0 : { *(.debug_ranges) }
  .debug_line    0 : { *(.debug_line) }
  .debug_str     0 : { *(.debug_str) }
}
