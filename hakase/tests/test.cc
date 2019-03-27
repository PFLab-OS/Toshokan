#include "test.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "_memory.h"
#include "channel2.h"

void pagetable_init() {
  uint64_t *mem = reinterpret_cast<uint64_t *>(DEPLOY_PHYS_ADDR_START);

  mem[static_cast<uint64_t>(MemoryMap::kPml4t) / sizeof(uint64_t)] = (static_cast<uint64_t>(MemoryMap::kPdpt) + DEPLOY_PHYS_ADDR_START) |
    (1 << 0) | (1 << 1) | (1 << 2);
  mem[static_cast<uint64_t>(MemoryMap::kPdpt) / sizeof(uint64_t)] = (static_cast<uint64_t>(MemoryMap::kPd) + DEPLOY_PHYS_ADDR_START) |
    (1 << 0) | (1 << 1) | (1 << 2);
  mem[static_cast<uint64_t>(MemoryMap::kPdpt) / sizeof(uint64_t) + 1] = (static_cast<uint64_t>(MemoryMap::kTmpPd) + DEPLOY_PHYS_ADDR_START) |
    (1 << 0) | (1 << 1) | (1 << 2);
  
  for (int i = 0; i < 512; i++) {
    mem[static_cast<uint64_t>(MemoryMap::kPd) / sizeof(uint64_t) + i] = (DEPLOY_PHYS_ADDR_START + (0x200000UL * i)) |
      (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);
  }
  
  mem[static_cast<uint64_t>(MemoryMap::kTmpPd) / sizeof(uint64_t)] = DEPLOY_PHYS_ADDR_START |
    (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);
}

int main(int argc, const char **argv) {
  FILE *cmdline_fp = fopen("/proc/cmdline", "r");
  if (!cmdline_fp) {
    perror("failed to open `cmdline`");
    return 255;
  }

  char buf[256];
  buf[fread(buf, 1, 255, cmdline_fp)] = '\0';
  if (!strstr(buf, "memmap=0x70000$4K memmap=0x40000000$0x40000000")) {
    fprintf(stderr, "error: physical memory is not isolated for toshokan.\n");
    return 255;
  }
 
  fclose(cmdline_fp);

  int mem_fd = open("/sys/module/friend_loader/call/mem", O_RDWR);
  if (mem_fd < 0) {
    perror("Open call failed");
    return 255;
  }
  char *mem =
      static_cast<char *>(mmap((void *)0x40000000UL, 0x40000000UL,
                               PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0));
  if (mem == MAP_FAILED) {
    perror("mmap operation failed...");
    return 255;
  }
  close(mem_fd);

  pagetable_init();
  munmap(mem, 0x40000000UL);

  int boot_fd = open("/sys/module/friend_loader/parameters/boot", O_RDWR);
  if (boot_fd < 0) {
    perror("failed to open `boot`");
    return 255;
  }

  if (write(boot_fd, "1", 2) != 2) {
    perror("write to `boot` failed");
    return 255;
  }

  int configfd_h2f = open("/sys/module/friend_loader/call/h2f", O_RDWR);
  int configfd_f2h = open("/sys/module/friend_loader/call/f2h", O_RDWR);
  int configfd_i2h = open("/sys/module/friend_loader/call/i2h", O_RDWR);
  if (configfd_h2f < 0 || configfd_f2h < 0 || configfd_i2h < 0) {
    perror("Open call failed");
    return 255;
  }

  char *h2f_address = static_cast<char *>(mmap(
      nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, configfd_h2f, 0));
  char *f2h_address = static_cast<char *>(mmap(
      nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, configfd_f2h, 0));
  char *i2h_address = static_cast<char *>(mmap(
      nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, configfd_i2h, 0));
  if (h2f_address == MAP_FAILED || f2h_address == MAP_FAILED ||
      i2h_address == MAP_FAILED) {
    perror("mmap operation failed");
    return 255;
  }
  F2H2 f2h(f2h_address);
  H2F2 h2f(h2f_address);
  I2H2 i2h(i2h_address);

  int rval = test_main(f2h, h2f, i2h, argc, argv);

  if (write(boot_fd, "0", 2) != 2) {
    perror("write to `boot` failed");
    return 255;
  }

  close(boot_fd);
  close(mem_fd);
  close(configfd_h2f);
  close(configfd_f2h);
  close(configfd_i2h);
  return rval;
}
