#include "int.h"
#include "trampoline.h"

struct idt_entity {
  uint32_t entry[4];
} __attribute__((aligned(8))) idt_def[Idt::kIntVectorNum];

volatile uint16_t _idtr[5];

extern idt_callback idt_vectors[Idt::kIntVectorNum];

extern Idt idt;

/* How to interrupt
 * 1. Trigger interrupt (e.g. "int" instruction, HW interrupt, some exception).
 * 2. All interruption invoke handle_int() with vector number. 
 * 3. handle_int() call a appropriate callback function or system halt if no callback function.
 *
 * Register Callback Function
 *  - Call SetIntCallback() or SetExceptionCallback()
 *    - Able to pass one argument, which is saved in IntCallback _callback[];
 */

namespace C {
extern "C" void handle_int(Regs *rs) {
  bool iflag = disable_interrupt();
  idt._handling_cnt++;
  if (idt._callback[rs->n].callback == nullptr) {
    //TODO notify to hakase 
    kassert(false);
  } else {
    idt._callback[rs->n].callback(rs, idt._callback[rs->n].arg);
  }
  idt._handling_cnt--;
  enable_interrupt(iflag);
}
}

void Idt::SetupGeneric() {
  for (int i = 0; i < kIntVectorNum; i++) {
    uint8_t ist;
    // We don't use TSS and IST
//    switch (i) {
//      case 8:
//        ist = 1;
//        break;
//      case 2:
//        ist = 2;
//        break;
//      case 1:
//      case 3:
//        ist = 3;
//        break;
//      case 18:
//        ist = 4;
//        break;
//      default:
//        ist = 5;
//        break;
//    };
    // mechanism of interrupt stack ref : SDM vol.3 6.14.4,5
    ist = 0;
    SetGate(idt_vectors[i], i, 0, false, ist);
  }

  // ref : SDM vol.2 LGDT/LIDT—Load Global/Interrupt Descriptor Table Register
  virt_addr idt_addr = reinterpret_cast<virt_addr>(idt_def);
  _idtr[0] = 0x10 * kIntVectorNum - 1;
  _idtr[1] = idt_addr & 0xffff;
  _idtr[2] = (idt_addr >> 16) & 0xffff;
  _idtr[3] = (idt_addr >> 32) & 0xffff;
  _idtr[4] = (idt_addr >> 48) & 0xffff;

  for (int i = 0; i < kIntVectorNum; i++) {
    _callback[i].callback = nullptr;
  }

  _is_gen_initialized = true;

}

void Idt::SetupProc() {
  asm volatile("lidt (%0)" ::"r"(_idtr));
  asm volatile("sti;");
}

void Idt::SetGate(idt_callback gate, int vector, uint8_t dpl, bool trap, uint8_t ist) {
  // structure of IDT ref: SDM vol.3 Figure 6-7
  virt_addr vaddr = reinterpret_cast<virt_addr>(gate);
  uint32_t type = trap ? 0xF : 0xE;
  idt_def[vector].entry[0] = (vaddr & 0xFFFF) | (KERNEL_CS << 16);
  idt_def[vector].entry[1] = (vaddr & 0xFFFF0000) | (type << 8) |
                             ((dpl & 0x3) << 13) | kIdtPresent | ist;
  idt_def[vector].entry[2] = vaddr >> 32;
  idt_def[vector].entry[3] = 0;

}

int Idt::SetIntCallback(int_callback callback, void *arg) {
  kassert(_is_gen_initialized);

  for (int vector = 32; vector < kIntVectorNum; vector++) {
    if (_callback[vector].callback == nullptr) {
      _callback[vector].callback = callback;
      _callback[vector].arg = arg;
      return vector;
    }
  }
  return ReservedIntVector::kError;
}

void Idt::SetExceptionCallback(int vector, int_callback callback, void *arg) {
  kassert(_is_gen_initialized);

  _callback[vector].callback = callback;
  _callback[vector].arg = arg;
}

