#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <toshokan/hakase/loader16.h>
#include <toshokan/memory.h>
#include <toshokan/symbol.h>
#include <unistd.h>

int Loader16::Init(uint64_t entry) {
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

  /*
    first 4KB of bootmem is for trampoline code
    remaining 12KB is for initial page tables
   */
  char *bootmem = static_cast<char *>(
      mmap(NULL, TRAMPOLINE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, bootmem_fd, 0));
  if (bootmem == MAP_FAILED) {
    perror("mmap operation failed...");
    return -1;
  }
  close(bootmem_fd);

  // copy friend binary to friend region + 8 byte
  memcpy(bootmem, _binary_hakase_friend16_bin_start,
         binary_hakase_friend16_bin_size);

  reinterpret_cast<uint64_t *>(bootmem)[1] = entry;

  // init page tables
  InitPageTables(bootmem + PAGE_SIZE);

  munmap(bootmem, TRAMPOLINE_SIZE);

  return 0;
}

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

extern Page SHARED_SYMBOL(__toshokan_pdpt);
extern Page SHARED_SYMBOL(__toshokan_pd1);
extern Page SHARED_SYMBOL(__toshokan_pd2);

void Loader16::InitPageTables(char *pml4t_addr) {
  // map trampoline region and friend memory region
  
  static const size_t k256TB = 256UL * 1024 * 1024 * 1024 * 1024;
  static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
  static const size_t k1GB = 1024UL * 1024 * 1024;
  static const size_t k2MB = 2UL * 1024 * 1024;

  Page *pml4t = (Page *)pml4t_addr;

  static const uint64_t kPageTableEntryFlags = (1 << 0) | (1 << 1) | (1 << 2);
  static const uint64_t kHugePageTableEntryFlags = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);

  static_assert((TRAMPOLINE_ADDR % k256TB) / k512GB == (DEPLOY_PHYS_ADDR_START % k256TB) / k512GB, "");
  
  pml4t->entry[(DEPLOY_PHYS_ADDR_START % k256TB) / k512GB] =
    reinterpret_cast<size_t>(&SHARED_SYMBOL(__toshokan_pdpt)) | kPageTableEntryFlags;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(TRAMPOLINE_ADDR % k512GB) / k1GB] =
    reinterpret_cast<size_t>(&SHARED_SYMBOL(__toshokan_pd1)) | kPageTableEntryFlags;
  SHARED_SYMBOL(__toshokan_pdpt).entry[(DEPLOY_PHYS_ADDR_START % k512GB) / k1GB] =
    reinterpret_cast<size_t>(&SHARED_SYMBOL(__toshokan_pd2)) | kPageTableEntryFlags;

  size_t trampoline_aligned_addr = (TRAMPOLINE_ADDR / k2MB) * k2MB;
  SHARED_SYMBOL(__toshokan_pd1).entry[(trampoline_aligned_addr % k1GB) / k2MB] =
      trampoline_aligned_addr | kHugePageTableEntryFlags;
  
  static_assert((DEPLOY_PHYS_ADDR_START % k1GB) == 0, "");
  static_assert(DEPLOY_PHYS_MEM_SIZE <= k1GB, "");
  for (size_t addr = DEPLOY_PHYS_ADDR_START; addr < DEPLOY_PHYS_ADDR_END;
       addr += k2MB) {
    SHARED_SYMBOL(__toshokan_pd2).entry[(addr % k1GB) / k2MB] =
      addr | kHugePageTableEntryFlags;
  }
}
