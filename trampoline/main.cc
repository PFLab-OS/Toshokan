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

// execute binary
void exec_bin(CalleeChannelAccessor &callee_ca) {
  uint64_t address =
      callee_ca.Read<uint64_t>(CalleeChannelAccessor::Offset<uint64_t>(0));
  // TODO check address
  callee_ca.Return(0);
  asm volatile("call *%0" ::"r"(address));
}

void rw_memory(CalleeChannelAccessor &callee_ca) {
  static const uint32_t kRead = 0;
  static const uint32_t kWrite = 1;

  uint32_t type =
      callee_ca.Read<uint32_t>(CalleeChannelAccessor::Offset<uint32_t>(0));
  uint64_t address_ =
      callee_ca.Read<uint64_t>(CalleeChannelAccessor::Offset<uint64_t>(8));
  uint64_t size =
      callee_ca.Read<uint64_t>(CalleeChannelAccessor::Offset<uint64_t>(16));

  if (address_ < DEPLOY_PHYS_ADDR_START || address_ + size > DEPLOY_PHYS_ADDR_END) {
    // avoid accessing to page unmapped region
    callee_ca.Return(-1);
    return;
  }

  uint8_t *address = reinterpret_cast<uint8_t *>(address_);
  if (type == kRead) {
    for (int i = 0; i < size; i++) {
      callee_ca.Write<uint8_t>(CalleeChannelAccessor::Offset<uint8_t>(i + 1024),
                               address[i]);
    }
  } else if (type == kWrite) {
    for (int i = 0; i < size; i++) {
      address[i] = callee_ca.Read<uint8_t>(
          CalleeChannelAccessor::Offset<uint8_t>(i + 1024));
    }
  } else {
    callee_ca.Return(-1);
    return;
  }
  callee_ca.Return(0);
}

static int32_t cpuid_cnt = 1;
extern "C" size_t stack_virt_addr;
size_t stack_virt_addr= DEPLOY_PHYS_ADDR_START + static_cast<size_t>(MemoryMap::kStack) + kStackSize;

extern "C" void trampoline_main() {
  int32_t cpuid = cpuid_cnt;
  cpuid_cnt++;
  stack_virt_addr += kStackSize;
  uint64_t *pc_st = reinterpret_cast<uint64_t *>(DEPLOY_PHYS_ADDR_START + static_cast<size_t>(MemoryMap::kPerCoreStruct));
  pc_st[cpuid] = cpuid;
  if (&pc_st[cpuid] >= reinterpret_cast<uint64_t *>(DEPLOY_PHYS_ADDR_START + static_cast<size_t>(MemoryMap::kEnd))) {
    panic();
  }

  if (cpuid == 1) {
    Channel2::InitBuffer(reinterpret_cast<uint8_t *>(DEPLOY_PHYS_ADDR_START + static_cast<size_t>(MemoryMap::kH2f)));
    Channel2::InitBuffer(reinterpret_cast<uint8_t *>(DEPLOY_PHYS_ADDR_START + static_cast<size_t>(MemoryMap::kF2h)));
    Channel2::InitBuffer(reinterpret_cast<uint8_t *>(DEPLOY_PHYS_ADDR_START + static_cast<size_t>(MemoryMap::kI2h)));
  }
  
  *reinterpret_cast<int32_t *>(DEPLOY_PHYS_ADDR_START + static_cast<size_t>(MemoryMap::kSync)) = cpuid;  // notify to FriendLoader
  asm volatile("wrmsr" ::"c"(0xC0000100 /* MSR_IA32_FS_BASE */), "d"(0),
               "a"(&pc_st[cpuid]));
  
  H2F2 h2f;
  F2H2 f2h;

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
  }
}
