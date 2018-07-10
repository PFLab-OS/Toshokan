#include "int.h"
#include "channel.h"

Idt idt;

static void HandleTest(Regs *rs, void *arg) {
  int *channel = reinterpret_cast<int *>(MemoryMap::kF2h);
  if (channel[0] == 0) {
    int16_t id;
    asm volatile("movw %%fs:0x0, %0" : "=r"(id));
    channel[0] = 7 | (id << 16);
  }
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

void exec_bin(H2F &h2f, F2H &f2h) {
  uint64_t address;
  h2f.Read(0, address);
  // TODO check address
  h2f.Return(0);
  asm volatile("call *%0"::"r"(address));
}

int main() {
  F2H f2h;
  f2h.Release();
  H2F h2f;
  h2f.Release();

  // Setup Interrupt 
  idt.SetupGeneric();
  idt.SetupProc();

  idt.SetExceptionCallback(Idt::ReservedIntVector::kTest, HandleTest, nullptr);

  // Load Elf
  f2h.Reserve();
  f2h.SendSignal(6);
  f2h.Release();


  while(true) {
    int16_t type;
    h2f.WaitNewSignal(type);

    if (type == 3) {
      exec_bin(h2f, f2h);
    }
    else if (type == 4) {
      rw_memory(h2f, f2h);
    }
  }

}
