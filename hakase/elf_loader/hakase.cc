#include "hakase.h"
#include "memrw/hakase.h"
#include <stdio.h>

std::unique_ptr<ElfLoader::ElfFile> ElfLoader::ElfFile::Load(const char *fname) {
  std::unique_ptr<ElfLoader::ElfFile> that(new ElfLoader::ElfFile);
  FILE *fp;

  fp = fopen(fname, "rb");
  if (fp == NULL){
    return std::unique_ptr<ElfLoader::ElfFile>();
  }

  while(true) {
    uint8_t buf[2048];
    size_t sz = fread(buf, 1, 2048, fp);
    if (sz == 0) {
      break;
    }
    that->_data.reserve(that->_data.size() + sz);
    for (size_t i = 0; i < sz; i++) {
      that->_data.push_back(buf[i]);
    }
  }
  
  fclose(fp);

  return that;
}

std::unique_ptr<Elf64_Shdr> GetShdr(int offset) {
  return nullptr;
}

Result<bool> ElfLoader::ElfFile::Init(H2F &_h2f) {
  if (_data.empty()) {
    return Result<bool>();
  }
  _ehdr.reset(new Ehdr(_data.data()));
  if (!_ehdr->IsElf() || !_ehdr->IsElf64() || (!_ehdr->IsOsabiSysv() && !_ehdr->IsOsabiGnu())) {
    return Result<bool>();
  }

  if (!_ehdr->IsExecutable()) {
    return Result<bool>();
  }

  return Result<bool>(true);
}

Result<bool> ElfLoader::Deploy() {
  // clear .bss section
  for(int i = 0; ; i++) {
    auto shdr = _file->GetShdr(i);
    if (!shdr) {
      break;
    }
    auto info = shdr->GetInfoIfBss();
    if (info) {
      if (info->vaddr < 1024 * 1024 /* 1MB */) {
        return Result<bool>();
      }
      uint8_t *buf = new uint8_t[info->size];
      memset(buf, 0, info->size);
      MemoryAccessor::Writer mw(_h2f, 1, info->vaddr, buf, info->size);
      mw.Do().Unwrap();
      delete buf;
    }
  }

  // load from file to memory
  for (int i = 0; ; i++) {
    auto phdr = _file->GetPhdr(i);
    if (!phdr) {
      break;
    }
    auto info = phdr->GetInfoIfLoad();
    if (info) {
      if (info->vaddr < 1024 * 1024 /* 1MB */) {
        return Result<bool>();
      }
      uint8_t *buf = new uint8_t[info->size];
      memcpy(buf, _file->GetRawPtr(info->file_offset), info->size);
      MemoryAccessor::Writer mw(_h2f, 1, info->vaddr, buf, info->size);
      mw.Do().Unwrap();
      delete buf;
    }
  }
  
  return Result<bool>(true);
}

Result<bool> ElfLoader::Execute(int16_t apicid) {
  Channel::Accessor ch_ac(_h2f, 3);
  ch_ac.Write(0, _file->GetEntry());
  if (ch_ac.Do(apicid) != 0) {
    return Result<bool>();
  }
  
  return Result<bool>(true);
}

