#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include "hakase/elfhead.h"
#include "hakase/loader16.h"
#include "memory.h"
#include "preallocated.h"
#include "result.h"

#include <memory>

// PIE is not supported
class ElfLoader {
 public:
  ElfLoader() = delete;
  ElfLoader(uint8_t *const addr, size_t len) : _addr(addr), _len(len) {}
  Result<bool> Deploy();
  Elf64_Off GetEntry() {
    Ehdr ehdr(_addr);
    return ehdr.GetEntry();
  }

 private:
  uint8_t *const _addr;
  size_t _len;

  Result<bool> CheckMemoryRegion(uint64_t vaddr, size_t size) {
    if (vaddr < DEPLOY_PHYS_ADDR_START || vaddr + size > DEPLOY_PHYS_ADDR_END) {
      // TODO show error
      return Result<bool>();
    } else {
      return Result<bool>(true);
    }
  }

  // wrapper of Elf64_Shdr
  class Shdr {
   public:
    Shdr(uint8_t *raw) : _raw(reinterpret_cast<Elf64_Shdr *>(raw)) {}
    Shdr() = delete;
    struct SectionInfo {
      uint64_t vaddr;
      size_t size;
    };
    std::unique_ptr<SectionInfo> GetInfoIfBss() {
      if (_raw->sh_type == SHT_NOBITS && (_raw->sh_flags & SHF_ALLOC) != 0) {
        auto info = std::unique_ptr<SectionInfo>(new SectionInfo);
        info->vaddr = _raw->sh_addr;
        info->size = _raw->sh_size;
        return info;
      }
      return std::unique_ptr<SectionInfo>();
    }

   private:
    Elf64_Shdr *_raw;
  };

  // wrapper of ELF64_Phdr
  class Phdr {
   public:
    Phdr(uint8_t *raw) : _raw(reinterpret_cast<Elf64_Phdr *>(raw)) {}
    Phdr() = delete;
    struct PhdrInfo {
      uint64_t vaddr;
      uint64_t file_offset;
      size_t size;
    };
    std::unique_ptr<PhdrInfo> GetInfoIfLoad() {
      if (_raw->p_type == PT_LOAD) {
        auto info = std::unique_ptr<PhdrInfo>(new PhdrInfo);
        info->vaddr = _raw->p_vaddr;
        info->file_offset = _raw->p_offset;
        info->size = _raw->p_filesz;
        return info;
      }
      return std::unique_ptr<PhdrInfo>();
    }

   private:
    Elf64_Phdr *_raw;
  };

  // wrapper of Elf64_Ehdr
  class Ehdr {
   public:
    Ehdr(uint8_t *raw) : _raw(reinterpret_cast<Elf64_Ehdr *>(raw)) {}
    Ehdr() = delete;
    bool IsElf() {
      return _raw->e_ident[0] == ELFMAG0 && _raw->e_ident[1] == ELFMAG1 &&
             _raw->e_ident[2] == ELFMAG2 && _raw->e_ident[3] == ELFMAG3;
    }
    bool IsElf64() { return _raw->e_ident[EI_CLASS] == ELFCLASS64; }
    bool IsOsabiSysv() { return _raw->e_ident[EI_OSABI] == ELFOSABI_SYSV; }
    bool IsOsabiGnu() { return _raw->e_ident[EI_OSABI] == ELFOSABI_GNU; }
    bool IsExecutable() { return _raw->e_type == ET_EXEC; }
    Elf64_Off GetEntry() { return _raw->e_entry; }
    Elf64_Off GetShdrOffset(int index) {
      if (_raw->e_shnum > index) {
        return _raw->e_shoff + _raw->e_shentsize * index;
      } else {
        return 0;
      }
    }
    Elf64_Off GetPhdrOffset(int index) {
      if (_raw->e_phnum > index) {
        return _raw->e_phoff + _raw->e_phentsize * index;
      } else {
        return 0;
      }
    }

   private:
    Elf64_Ehdr *_raw;
  };

  std::unique_ptr<Shdr> CreateShdr(Elf64_Off offset) {
    if (offset == 0) {
      return std::unique_ptr<Shdr>();
    } else {
      return std::unique_ptr<Shdr>(new Shdr(_addr + offset));
    }
  }
  std::unique_ptr<Phdr> CreatePhdr(Elf64_Off offset) {
    if (offset == 0) {
      return std::unique_ptr<Phdr>();
    } else {
      return std::unique_ptr<Phdr>(new Phdr(_addr + offset));
    }
  }
};

Result<bool> ElfLoader::Deploy() {
  Ehdr ehdr(_addr);
  if (!ehdr.IsElf() || !ehdr.IsElf64() ||
      (!ehdr.IsOsabiSysv() && !ehdr.IsOsabiGnu())) {
    std::cerr << "ElfLoader: error: unknown binary!" << std::endl;
    return Result<bool>();
  }

  if (!ehdr.IsExecutable()) {
    std::cerr << "ElfLoader: error: unknown binary!" << std::endl;
    return Result<bool>();
  }

  // clear .bss section
  for (int i = 0;; i++) {
    auto shdr = CreateShdr(ehdr.GetShdrOffset(i));
    if (!shdr) {
      break;
    }
    auto info = shdr->GetInfoIfBss();
    if (info) {
      if (CheckMemoryRegion(info->vaddr, info->size).IsError()) {
        std::cerr
            << "ElfLoader: error: unable to deploy outside of friend memory!"
            << std::endl;
        return Result<bool>();
      }
      memset(reinterpret_cast<void *>(info->vaddr), 0, info->size);
    }
  }

  // load from file to memory
  for (int i = 0;; i++) {
    auto phdr = CreatePhdr(ehdr.GetPhdrOffset(i));
    if (!phdr) {
      break;
    }
    auto info = phdr->GetInfoIfLoad();
    if (info) {
      if (CheckMemoryRegion(info->vaddr, info->size).IsError()) {
        std::cerr
            << "ElfLoader: error: unable to deploy outside of friend memory!"
            << std::endl;
        return Result<bool>();
      }
      memcpy(reinterpret_cast<void *>(info->vaddr), _addr + info->file_offset,
             info->size);
    }
  }

  return Result<bool>(true);
}

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
    std::cerr << "error: physical memory is not isolated for toshokan."
              << std::endl;
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
  Page *pml4t = &preallocated_mem->pml4t;
  Page *pdpt = &preallocated_mem->pdpt;
  Page *pd = &preallocated_mem->pd;

  // TODO: refactor this(not to use offset, but structure)
  pml4t->entry[(DEPLOY_PHYS_ADDR_START % k256TB) / k512GB] =
      reinterpret_cast<size_t>(pdpt) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt->entry[(DEPLOY_PHYS_ADDR_START % k512GB) / k1GB] =
      reinterpret_cast<size_t>(pd) | (1 << 0) | (1 << 1) | (1 << 2);

  static_assert((DEPLOY_PHYS_ADDR_START % k1GB) == 0, "");
  static_assert(DEPLOY_PHYS_MEM_SIZE <= k1GB, "");
  for (size_t addr = DEPLOY_PHYS_ADDR_START; addr < DEPLOY_PHYS_ADDR_END;
       addr += k2MB) {
    pd->entry[(addr % k1GB) / k2MB] =
        addr | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);
  }
}

extern "C" void entry32();
int main(int argc, const char **argv) {
  extern uint8_t _binary_tests_elf_friend_bin_start[];
  extern uint8_t _binary_tests_elf_friend_bin_size[];
  size_t binary_tests_elf_friend_bin_size =
      reinterpret_cast<size_t>(_binary_tests_elf_friend_bin_size);

  Loader16 loader16;
  ElfLoader elfloader(_binary_tests_elf_friend_bin_start,
                      binary_tests_elf_friend_bin_size);

  if (check_bootparam() < 0) {
    return 255;
  }

  assert(friend_mem_start == reinterpret_cast<char *>(DEPLOY_PHYS_ADDR_START));
  assert(friend_mem_end == reinterpret_cast<char *>(DEPLOY_PHYS_ADDR_END));

  if (mmap_friend_mem() < 0) {
    return 255;
  }

  if (elfloader.Deploy().IsError()) {
    std::cerr << "error: failed to deploy elf binary" << std::endl;
    return 255;
  }

  Elf64_Off entry = elfloader.GetEntry();
  assert(entry < 0xFFFFFFFF);
  if (loader16.Init(entry) < 0) {
    std::cerr << "error: failed to init friend16 region" << std::endl;
    return 255;
  }
  assert(entry == (Elf64_Off)entry32);

  pagetable_init();

  int cpunum = 0;

  for (int i = 1;; i++) {
    char buf[20];
    sprintf(buf, "/dev/friend_cpu%d", i);
    if (open(buf, O_RDONLY) < 0) {
      cpunum = i - 1;
      break;
    }
  }

  sleep(1);

  return (preallocated_mem->sync_flag == cpunum) ? 0 : 255;
}
