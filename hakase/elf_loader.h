#pragma once
#include <stddef.h>
#include <stdint.h>
#include <toshokan/elfhead.h>
#include <toshokan/memory.h>
#include <memory>

// PIE is not supported
class ElfLoader {
 public:
  ElfLoader() = delete;
  ElfLoader(uint8_t *const addr, size_t len) : _addr(addr), _len(len) {}
  __attribute__((warn_unused_result)) bool Deploy();
  Elf64_Off GetEntry() {
    Ehdr ehdr(_addr);
    return ehdr.GetEntry();
  }

 private:
  uint8_t *const _addr;
  size_t _len;

  __attribute__((warn_unused_result)) bool CheckMemoryRegion(uint64_t vaddr,
                                                             size_t size) {
    if (vaddr < DEPLOY_PHYS_ADDR_START || vaddr + size > DEPLOY_PHYS_ADDR_END) {
      return false;
    } else {
      return true;
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

    std::unique_ptr<SectionInfo> GetInfoIfExec() {
      if ((_raw->sh_flags & SHF_EXECINSTR) != 0) {
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
