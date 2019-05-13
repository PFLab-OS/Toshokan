SECTIONS {
  . = 0x40000000;
  .friend_mem  : {
    friend_mem_start = .;
    . += 0x40000000;
    friend_mem_end = .;
  }
}
