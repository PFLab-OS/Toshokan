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
uint64_t *mem = reinterpret_cast<uint64_t *>(friend_mem_start);

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
  // TODO: refactor this(not to use offset, but structure)
  mem[static_cast<uint64_t>(MemoryMap::kPml4t) / sizeof(uint64_t)] =
      (static_cast<uint64_t>(MemoryMap::kPdpt) + DEPLOY_PHYS_ADDR_START) |
      (1 << 0) | (1 << 1) | (1 << 2);
  mem[static_cast<uint64_t>(MemoryMap::kPdpt) / sizeof(uint64_t)] =
      (static_cast<uint64_t>(MemoryMap::kPd) + DEPLOY_PHYS_ADDR_START) |
      (1 << 0) | (1 << 1) | (1 << 2);
  mem[static_cast<uint64_t>(MemoryMap::kPdpt) / sizeof(uint64_t) + 1] =
      (static_cast<uint64_t>(MemoryMap::kTmpPd) + DEPLOY_PHYS_ADDR_START) |
      (1 << 0) | (1 << 1) | (1 << 2);

  for (int i = 0; i < 512; i++) {
    mem[static_cast<uint64_t>(MemoryMap::kPd) / sizeof(uint64_t) + i] =
        (DEPLOY_PHYS_ADDR_START + (0x200000UL * i)) | (1 << 0) | (1 << 1) |
        (1 << 2) | (1 << 7);
  }

  mem[static_cast<uint64_t>(MemoryMap::kTmpPd) / sizeof(uint64_t)] =
      DEPLOY_PHYS_ADDR_START | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);
}

static uint64_t add_base_addr_to_segment_descriptor(uint64_t desc) {
  return desc | ((DEPLOY_PHYS_ADDR_START & 0xFFFFFF) << 16) |
         ((DEPLOY_PHYS_ADDR_START >> 24) << 56);
}

int trampoline_region_init() {
  extern uint8_t _binary_boot_trampoline_bin_start[];
  extern uint8_t _binary_boot_trampoline_bin_end[];
  extern uint8_t _binary_boot_trampoline_bin_size[];
  size_t binary_boot_trampoline_bin_size =
      (size_t)_binary_boot_trampoline_bin_size;
  const size_t kRegionSize =
      binary_boot_trampoline_bin_size +
      static_cast<uint64_t>(MemoryMap::kTrampolineBinLoadPoint);

  static uint8_t jmp_bin[] = {
      0xeb, static_cast<uint64_t>(MemoryMap::kTrampolineBinEntry) - 2, 0x66,
      0x90};  // jmp TrampolineBinEntry; xchg %ax, &ax
  memcpy(mem, jmp_bin, sizeof(jmp_bin) / sizeof(jmp_bin[0]));

  if (PAGE_SIZE < kRegionSize) {
    // trampoline code is so huge
    return -1;
  }

  if (_binary_boot_trampoline_bin_start + binary_boot_trampoline_bin_size !=
      _binary_boot_trampoline_bin_end) {
    // impossible...
    return -1;
  }

  // copy trampoline binary to trampoline region + 8 byte
  memcpy(reinterpret_cast<uint8_t *>(mem) +
             static_cast<uint64_t>(MemoryMap::kTrampolineBinLoadPoint),
         _binary_boot_trampoline_bin_start, binary_boot_trampoline_bin_size);

  // initialize trampoline header
  mem[static_cast<uint64_t>(MemoryMap::kPhysAddrStart) / sizeof(*mem)] =
      DEPLOY_PHYS_ADDR_START;
  // null descriptor
  mem[static_cast<uint64_t>(MemoryMap::kGdtPtr32) / sizeof(*mem) + 0] = 0;
  // kernel code descriptor
  mem[static_cast<uint64_t>(MemoryMap::kGdtPtr32) / sizeof(*mem) + 1] =
      add_base_addr_to_segment_descriptor(0x00CF9A000000FFFFUL);
  // kernel data descriptor
  mem[static_cast<uint64_t>(MemoryMap::kGdtPtr32) / sizeof(*mem) + 2] =
      add_base_addr_to_segment_descriptor(0x00CF92000000FFFFUL);
  mem[static_cast<uint64_t>(MemoryMap::kId) / sizeof(*mem)] =
      0;  // will be initialized by trampoline_region_set_id()
  mem[static_cast<uint64_t>(MemoryMap::kStackVirtAddr) / sizeof(*mem)] =
      0;  // will be initialized by trampoline_region_set_id()

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

  memcpy(bootmem, mem, kRegionSize);

  munmap(bootmem, PAGE_SIZE);

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

  if (trampoline_region_init() < 0) {
    fprintf(stderr, "error: failed to init trampoline region\n");
    return 255;
  }

  int boot_fd = open("/sys/module/friend_loader/parameters/boot", O_RDWR);
  if (boot_fd < 0) {
    perror("failed to open `boot`");
    return 255;
  }

  // TODO: fix static ID
  int32_t id_buf[2];
  id_buf[0] = 1;  // TODO: apicid
  id_buf[1] = 1;  // TODO: cpuid
  uint64_t stack_addr = 1 /* TODO: cpuid */ * kStackSize +
                        static_cast<uint64_t>(MemoryMap::kStack);
  memcpy(mem + static_cast<uint64_t>(MemoryMap::kId) / sizeof(uint64_t), id_buf,
         sizeof(id_buf));
  mem[static_cast<uint64_t>(MemoryMap::kStackVirtAddr) / sizeof(uint64_t)] =
      stack_addr;

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
  close(configfd_h2f);
  close(configfd_f2h);
  close(configfd_i2h);
  return rval;
}
