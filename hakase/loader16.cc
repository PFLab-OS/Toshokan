#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <toshokan/hakase/loader16.h>
#include <toshokan/memory.h>
#include <unistd.h>

int Loader16::Init(uint32_t entry) {
  extern uint8_t _binary_hakase_friend16_bin_start[];
  extern uint8_t _binary_hakase_friend16_bin_size[];
  const size_t binary_hakase_friend16_bin_size =
      reinterpret_cast<size_t>(_binary_hakase_friend16_bin_size);

  if (binary_hakase_friend16_bin_size > PAGE_SIZE) {
    // friend code is so huge
    return -1;
  }

  int bootmem_fd = open("/dev/friend_bootmem" TRAMPOLINE_ADDR_STR, O_RDWR);
  if (bootmem_fd < 0) {
    perror("Open call failed");
    return -1;
  }
  char *bootmem = static_cast<char *>(
      mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, bootmem_fd, 0));
  if (bootmem == MAP_FAILED) {
    perror("mmap operation failed...");
    return -1;
  }
  close(bootmem_fd);

  // copy friend binary to friend region + 8 byte
  memcpy(bootmem, _binary_hakase_friend16_bin_start,
         binary_hakase_friend16_bin_size);

  reinterpret_cast<uint32_t *>(bootmem)[1] = entry;

  munmap(bootmem, PAGE_SIZE);

  return 0;
}
