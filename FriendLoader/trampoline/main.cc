#include "channel.h"

void panic();

// callback test
void callback(H2F &h2f, F2H &f2h) {
  h2f.Return(0);
  f2h.Reserve();
  f2h.SendSignal(1);
  f2h.Release();
}

// print request test
void print(H2F &h2f, F2H &f2h) {
  h2f.Return(0);
  f2h.WriteString("abc\n");
}

// execute binary
void exec_bin(H2F &h2f, F2H &f2h) {
  uint64_t address;
  h2f.Read(0, address);
  // TODO check address
  h2f.Return(0);
  asm volatile("call *%0"::"r"(address));
}

void rw_memory(H2F &h2f, F2H &f2h) {
  static const uint32_t kRead = 0;
  static const uint32_t kWrite = 1;
  
  uint32_t type;
  uint64_t address_;
  uint64_t size;
  uint8_t *buf = h2f.GetRawPtr<uint8_t>() + 2048;
  
  h2f.Read(0, type);
  h2f.Read(8, address_);
  h2f.Read(16, size);

  if (address_ + 2048 / sizeof(uint64_t) >= 1024 * 1024 * 1024 /* 1GB */) {
    // avoid accessing to page unmapped region
    h2f.Return(-1);
    return;
  }
  
  uint8_t *address = reinterpret_cast<uint8_t *>(address_);
  if (type == kRead) {
    for(int i = 0; i < size; i++) {
      buf[i] = address[i];
    }
  } else if (type == kWrite) {
    for(int i = 0; i < size; i++) {
      address[i] = buf[i];
    }
  } else {
    h2f.Return(-1);
    return;
  }
  
  h2f.Return(0);
}

extern "C" void trampoline_main() {
  uint64_t *pml4t = reinterpret_cast<uint64_t *>(MemoryMap::kPml4t);
  uint64_t *pdpt  = reinterpret_cast<uint64_t *>(MemoryMap::kPdpt);
  uint64_t *pd    = reinterpret_cast<uint64_t *>(MemoryMap::kPd);

  pml4t[0] = (reinterpret_cast<uint64_t>(pdpt) + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[0]  = (reinterpret_cast<uint64_t>(pd) + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  for(int i = 0; i < 512; i++) {
    pd[i]  = (0x80000000UL + (0x200000UL * i)) | (1 << 0) | (1 << 1) | (1 << 2) | (1<<7);
  }
  asm volatile("movq %0, %%cr3;"::"r"(reinterpret_cast<uint64_t>(pml4t) + 0x80000000UL));
  
  H2F h2f;
  F2H f2h;

  while(true) {
    int16_t type;
    h2f.WaitNewSignal(type);
    switch (type) {
    case 0:
      panic();
    case 1:
      callback(h2f, f2h);
      break;
    case 2:
      print(h2f, f2h);
      break;
    case 3:
      exec_bin(h2f, f2h);
      break;
    case 4:
      rw_memory(h2f, f2h);
      break;
    }
  }
}

void panic() {
  while (true) {
    asm volatile("cli;hlt;nop;");
  }
}
