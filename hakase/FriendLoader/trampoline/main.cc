#include "channel.h"
#include "panic.h"

// callback test
void callback(H2F &h2f, F2H &f2h) {
  h2f.Return(0);
  f2h.Reserve(0);
  f2h.SendSignal(1);
  f2h.Release();
}

// print request test
void print(H2F &h2f, F2H &f2h) {
  h2f.Return(0);
  f2h.WriteString(0, "abc\n");
}

// execute binary
void exec_bin(H2F &h2f, F2H &f2h) {
  uint64_t address = h2f.OldRead<uint64_t>(0);
  // TODO check address
  h2f.Return(0);
  asm volatile("call *%0" ::"r"(address));
}

void rw_memory(H2F &h2f, F2H &f2h) {
  static const uint32_t kRead = 0;
  static const uint32_t kWrite = 1;

  uint32_t type = h2f.OldRead<uint32_t>(0);
  uint64_t address_ = h2f.OldRead<uint64_t>(8);
  uint64_t size = h2f.OldRead<uint64_t>(16);

  if (address_ + 2048 / sizeof(uint64_t) >= 1024 * 1024 * 1024 /* 1GB */) {
    // avoid accessing to page unmapped region
    h2f.Return(-1);
    return;
  }

  uint8_t *address = reinterpret_cast<uint8_t *>(address_);
  if (type == kRead) {
    for (int i = 0; i < size; i++) {
      h2f.Write(i + 2040, address[i]);
    }
  } else if (type == kWrite) {
    for (int i = 0; i < size; i++) {
      address[i] = h2f.Read(i + 2040);
    }
  } else {
    h2f.Return(-1);
    return;
  }

  h2f.Return(0);
}

extern "C" void trampoline_main() {
  uint32_t *id = reinterpret_cast<uint32_t *>(MemoryMap::kId);
  int32_t cpuid = id[1];
  uint64_t *pc_st = reinterpret_cast<uint64_t *>(MemoryMap::kPerCoreStruct);
  pc_st[cpuid] = *reinterpret_cast<uint64_t *>(id);
  if (&pc_st[cpuid] >= reinterpret_cast<uint64_t *>(MemoryMap::kEnd)) {
    panic();
  }
  *reinterpret_cast<uint64_t *>(id) = 0;  // notify to FriendLoader
  asm volatile("wrmsr" ::"c"(0xC0000100 /* MSR_IA32_FS_BASE */), "d"(0),
               "a"(&pc_st[cpuid]));

  H2F h2f;
  F2H f2h;

  while (true) {
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
