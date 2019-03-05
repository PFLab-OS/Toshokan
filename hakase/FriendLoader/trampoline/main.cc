#include "channel.h"
#include "channel_accessor2.h"
#include "panic.h"

// callback test
void callback(CalleeChannelAccessor &callee_ca, F2H2 &f2h) {
  Channel2::Id id = callee_ca.GetId();
  callee_ca.Return(0);
  CallerChannelAccessor caller_ca(f2h, id, Channel2::Signal::kCallback());
  caller_ca.Call();
}

// print request test
void print(H2F &h2f, F2H &f2h) {
  h2f.Return(0);
  f2h.WriteString(0, "abc\n");
}

// execute binary
void exec_bin(CalleeChannelAccessor &callee_ca) {
  uint64_t address = callee_ca.Read<uint64_t>(CalleeChannelAccessor::Offset<uint64_t>(0));
  // TODO check address
  callee_ca.Return(0);
  asm volatile("call *%0" ::"r"(address));
}

void rw_memory(CalleeChannelAccessor &callee_ca) {
  static const uint32_t kRead = 0;
  static const uint32_t kWrite = 1;

  uint32_t type = callee_ca.Read<uint32_t>(CalleeChannelAccessor::Offset<uint32_t>(0));
  uint64_t address_ = callee_ca.Read<uint64_t>(CalleeChannelAccessor::Offset<uint64_t>(8));
  uint64_t size = callee_ca.Read<uint64_t>(CalleeChannelAccessor::Offset<uint64_t>(16));

  if (address_ + 2048 / sizeof(uint64_t) >= 1024 * 1024 * 1024 /* 1GB */) {
    // avoid accessing to page unmapped region
    callee_ca.Return(-1);
    return;
  }

  uint8_t *address = reinterpret_cast<uint8_t *>(address_);
  if (type == kRead) {
    for (int i = 0; i < size; i++) {
      callee_ca.Write<uint8_t>(CalleeChannelAccessor::Offset<uint8_t>(i + 1024), address[i]);
    }
  } else if (type == kWrite) {
    for (int i = 0; i < size; i++) {
      address[i] = callee_ca.Read<uint8_t>(CalleeChannelAccessor::Offset<uint8_t>(i + 1024));
    }
  } else {
    callee_ca.Return(-1);
    return;
  }
  callee_ca.Return(0);
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

  H2F2 h2f;
  F2H2 f2h;

  if (cpuid == 1) {
    Channel2::InitBuffer(reinterpret_cast<uint8_t *>(MemoryMap::kH2f));
    Channel2::InitBuffer(reinterpret_cast<uint8_t *>(MemoryMap::kF2h));
    Channel2::InitBuffer(reinterpret_cast<uint8_t *>(MemoryMap::kI2h));
  }

  while (true) {
    CalleeChannelAccessor callee_ca(h2f);
    callee_ca.ReceiveSignal();
    Channel2::Signal signal = callee_ca.GetSignal();
    if (signal == Channel2::Signal::kCallback()) {
      callback(callee_ca, f2h);
    } else if (signal == Channel2::Signal::kRwMemory()) {
      rw_memory(callee_ca);
    } else if (signal == Channel2::Signal::kExec()) {
      exec_bin(callee_ca);
    }
      

    // int16_t type;
    // h2f.WaitNewSignal(type);
    // switch (type) {
    //   case 0:
    //     panic();
    //   case 1:
    //     callback(h2f, f2h);
    //     break;
    //   case 2:
    //     print(h2f, f2h);
    //     break;
    //   case 3:
    //     exec_bin(h2f, f2h);
    //     break;
    //   case 4:
    //     rw_memory(h2f, f2h);
    //     break;
    // }
  }
}
