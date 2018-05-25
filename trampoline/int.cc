#include "int.h"
#include "trampoline.h"

struct idt_entity {
  uint32_t entry[4];
} __attribute__((aligned(8))) idt_def[Idt::kIntVectorNum];

volatile uint16_t _idtr[5];

extern idt_callback idt_vectors[Idt::kIntVectorNum];

namespace C {
extern "C" void handle_int(Regs *rs) {
}
}

void Idt::SetupGeneric() {
  for (int i = 0; i < kIntVectorNum; i++) {
    uint8_t ist;
    switch (i) {
      case 8:
        ist = 1;
        break;
      case 2:
        ist = 2;
        break;
      case 1:
      case 3:
        ist = 3;
        break;
      case 18:
        ist = 4;
        break;
      default:
        ist = 5;
        break;
    };
    SetGate(idt_vectors[i], i, 0, false, ist);
  }
  virt_addr idt_addr = reinterpret_cast<virt_addr>(idt_def);
  _idtr[0] = 0x10 * kIntVectorNum - 1;
  _idtr[1] = idt_addr & 0xffff;
  _idtr[2] = (idt_addr >> 16) & 0xffff;
  _idtr[3] = (idt_addr >> 32) & 0xffff;
  _idtr[4] = (idt_addr >> 48) & 0xffff;

  for (int i = 0; i < kIntVectorNum; i++) {
    _callback[i].callback = nullptr;
    _callback[i].eoi = EoiType::kNone;
  }

  _is_gen_initialized = true;

}

void Idt::SetupProc() {
  asm volatile("lidt (%0)" ::"r"(_idtr));
  asm volatile("sti;");
}

void Idt::SetGate(idt_callback gate, int vector, uint8_t dpl, bool trap,
                  uint8_t ist) {
  virt_addr vaddr = reinterpret_cast<virt_addr>(gate);
  uint32_t type = trap ? 0xF : 0xE;
  idt_def[vector].entry[0] = (vaddr & 0xFFFF) | (KERNEL_CS << 16);
  idt_def[vector].entry[1] = (vaddr & 0xFFFF0000) | (type << 8) |
                             ((dpl & 0x3) << 13) | kIdtPresent; // | ist; TODO: using IST
  idt_def[vector].entry[2] = vaddr >> 32;
  idt_def[vector].entry[3] = 0;

}

int Idt::SetIntCallback(int_callback callback, void *arg,
                        EoiType eoi) {
  kassert(_is_gen_initialized);

  for (int vector = 64; vector < 256; vector++) {
    if (_callback[vector].callback == nullptr) {
      _callback[vector].callback = callback;
      _callback[vector].arg = arg;
      _callback[vector].eoi = eoi;
      return vector;
    }
  }
  return ReservedIntVector::kError;
}

int Idt::SetIntCallback(int_callback *callback, void **arg,
                        int range, EoiType eoi) {
  kassert(_is_gen_initialized);
  int _range = 1;
  while (_range < range) {
    _range *= 2;
  }
  if (range != _range) {
    return ReservedIntVector::kError;
  }
  int vector = range > 64 ? range : 64;
  for (; vector < 256; vector += range) {
    int i;
    for (i = 0; i < range; i++) {
      if (_callback[vector + i].callback != nullptr) {
        break;
      }
    }
    if (i != range) {
      continue;
    }
    for (i = 0; i < range; i++) {
      _callback[vector + i].callback = callback[i];
      _callback[vector + i].arg = arg[i];
      _callback[vector + i].eoi = eoi;
    }
    return vector;
  }
  return ReservedIntVector::kError;
}
