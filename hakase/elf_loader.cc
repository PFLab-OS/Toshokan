#include <toshokan/hakase/elf_loader.h>

bool ElfLoader::Deploy() {
  Ehdr ehdr(_addr);
  if (!ehdr.IsElf() || !ehdr.IsElf64() ||
      (!ehdr.IsOsabiSysv() && !ehdr.IsOsabiGnu())) {
    return false;
  }

  if (!ehdr.IsExecutable()) {
    return false;
  }

  // clear .bss section
  for (int i = 0;; i++) {
    auto shdr = CreateShdr(ehdr.GetShdrOffset(i));
    if (!shdr) {
      break;
    }
    auto info = shdr->GetInfoIfBss();
    if (info) {
      if (!CheckMemoryRegion(info->vaddr, info->size)) {
        return false;
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
      if (!CheckMemoryRegion(info->vaddr, info->size)) {
        return false;
      }
      memcpy(reinterpret_cast<void *>(info->vaddr), _addr + info->file_offset,
             info->size);
    }
  }

  return true;
}
