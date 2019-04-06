#include "test.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "_memory.h"
#include "channel2.h"

extern char friend_mem_start[];
extern char friend_mem_end[];
uint64_t *const mem = reinterpret_cast<uint64_t *>(friend_mem_start);

int check_bootparam() {
  FILE *cmdline_fp = fopen("/proc/cmdline", "r");
  if (!cmdline_fp) {
    perror("failed to open `cmdline`");
    return -1;
  }

  char buf[256];
  buf[fread(buf, 1, 255, cmdline_fp)] = '\0';
  if (!strstr(buf, "memmap=0x70000$4K memmap=0x40000000$0x40000000")) {
    fprintf(stderr, "error: physical memory is not isolated for toshokan.\n");
    return -1;
  }

  fclose(cmdline_fp);

  return 0;
}

int mmap_friend_mem() {
  int mem_fd = open("/sys/module/friend_loader/call/mem", O_RDWR);
  if (mem_fd < 0) {
    perror("Open call failed");
    return -1;
  }

  void *mmapped_addr = mmap(mem, DEPLOY_PHYS_MEM_SIZE, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_FIXED, mem_fd, 0);
  if (mmapped_addr == MAP_FAILED) {
    perror("mmap operation failed...");
    return -1;
  }
  assert(reinterpret_cast<void *>(mem) == mmapped_addr);

  close(mem_fd);

  // zero clear (only 4MB, because it is too slow to clear whole memory)
  memset(mem, 0, 1024 * 4096);
  return 0;
}

void pagetable_init() {
  static const size_t k256TB = 256UL * 1024 * 1024 * 1024 * 1024;
  static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
  static const size_t k1GB = 1024UL * 1024 * 1024;
  static const size_t k2MB = 2UL * 1024 * 1024;
  uint64_t *const pml4t =
      &mem[static_cast<uint64_t>(MemoryMap::kPml4t) / sizeof(uint64_t)];
  uint64_t *const pdpt =
      &mem[static_cast<uint64_t>(MemoryMap::kPdpt) / sizeof(uint64_t)];
  uint64_t *const pd =
      &mem[static_cast<uint64_t>(MemoryMap::kPd) / sizeof(uint64_t)];

  // TODO: refactor this(not to use offset, but structure)
  pml4t[(DEPLOY_PHYS_ADDR_START % k256TB) / k512GB] =
      reinterpret_cast<size_t>(pdpt) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[(DEPLOY_PHYS_ADDR_START % k512GB) / k1GB] =
      reinterpret_cast<size_t>(pd) | (1 << 0) | (1 << 1) | (1 << 2);

  static_assert((DEPLOY_PHYS_ADDR_START % k1GB) == 0, "");
  static_assert(DEPLOY_PHYS_MEM_SIZE <= k1GB, "");
  for (size_t addr = DEPLOY_PHYS_ADDR_START; addr < DEPLOY_PHYS_ADDR_END;
       addr += k2MB) {
    pd[(addr % k1GB) / k2MB] = addr | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);
  }
}

int trampoline16_region_init() {
  extern uint8_t _binary_boot_trampoline16_bin_start[];
  extern uint8_t _binary_boot_trampoline16_bin_size[];
  const size_t binary_boot_trampoline16_bin_size =
      reinterpret_cast<size_t>(_binary_boot_trampoline16_bin_size);

  if (binary_boot_trampoline16_bin_size > PAGE_SIZE) {
    // trampoline code is so huge
    return -1;
  }

  int bootmem_fd =
      open("/sys/module/friend_loader/call/" TRAMPOLINE_ADDR_STR, O_RDWR);
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

  // copy trampoline binary to trampoline region + 8 byte
  memcpy(bootmem, _binary_boot_trampoline16_bin_start,
         binary_boot_trampoline16_bin_size);

  munmap(bootmem, PAGE_SIZE);

  return 0;
}

int trampoline_region_init() {
  extern uint8_t _binary_boot_trampoline_bin_start[];
  extern uint8_t _binary_boot_trampoline_bin_size[];
  size_t binary_boot_trampoline_bin_size =
      reinterpret_cast<size_t>(_binary_boot_trampoline_bin_size);

  if (binary_boot_trampoline_bin_size >
      static_cast<size_t>(MemoryMap::kPml4t)) {
    // trampoline code is so huge
    return -1;
  }

  // copy trampoline binary
  memcpy(mem, _binary_boot_trampoline_bin_start,
         binary_boot_trampoline_bin_size);

  return 0;
}

int main(int argc, const char **argv) {
  if (check_bootparam() < 0) {
    return 255;
  }

  assert(friend_mem_start == reinterpret_cast<char *>(DEPLOY_PHYS_ADDR_START));
  assert(friend_mem_end == reinterpret_cast<char *>(DEPLOY_PHYS_ADDR_END));

  if (mmap_friend_mem() < 0) {
    return 255;
  }

  pagetable_init();

  if (trampoline16_region_init() < 0) {
    fprintf(stderr, "error: failed to init trampoline16 region\n");
    return 255;
  }

  if (trampoline_region_init() < 0) {
    fprintf(stderr, "error: failed to init trampoline region\n");
    return 255;
  }

  for (int i = 1;; i++) {
    mem[static_cast<uint64_t>(MemoryMap::kSync) / sizeof(uint64_t)] = 0;

    char buf[20];
    sprintf(buf, "/dev/friend_cpu%d", i);
    if (open(buf, O_RDONLY) < 0) {
      break;
    }

    do {
      // wait until kMemoryMapId is written by a friend.
      if (reinterpret_cast<int32_t *>(
              mem)[static_cast<uint64_t>(MemoryMap::kSync) / sizeof(int32_t)] ==
          i) {
        break;
      }
      asm volatile("pause" ::: "memory");
    } while (1);
  }

  H2F2 h2f(reinterpret_cast<char *>(DEPLOY_PHYS_ADDR_START +
                                    static_cast<size_t>(MemoryMap::kH2f)));
  F2H2 f2h(reinterpret_cast<char *>(DEPLOY_PHYS_ADDR_START +
                                    static_cast<size_t>(MemoryMap::kF2h)));
  I2H2 i2h(reinterpret_cast<char *>(DEPLOY_PHYS_ADDR_START +
                                    static_cast<size_t>(MemoryMap::kI2h)));

  return test_main(f2h, h2f, i2h, argc, argv);
}
