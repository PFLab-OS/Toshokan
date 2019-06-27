#include <toshokan/friend/export.h>
#include <toshokan/friend/offload.h>
#include "int.h"
#include "shared.h"

Idt idt;

int SHARED_SYMBOL(rval) = 0;
int64_t SHARED_SYMBOL(sync_flag) = 0;
int (*EXPORTED_SYMBOL(printf))(const char *format, ...);

static void handler(Regs *rs, void *arg) { SHARED_SYMBOL(rval) = 1; }

// masking function
// must be b1 > b2
template <class T>
T mask(T val, int b1, int b2) {
  return (val >> b2) & ((1UL << (b1 - b2 + 1)) - 1);
}

template <class T>
bool get_bit(T val, int b) {
  return ((val & (1 << b)) != 0);
}

void friend_main() {
  idt.SetupGeneric();
  idt.SetupProc();
  int vector = idt.SetIntCallback(handler, nullptr);
  OFFLOAD_FUNC(printf, "set handler at vector %d\n", vector);
  *((uint32_t *)0xfee000f0) =
      (1 << 8);  // enable local apic (WARNING: too rough. read Intel SDM)
  enable_interrupt(true);

  uint64_t capability = *((uint64_t *)0xfed00000UL);
  uint64_t counter_clk_num_per_ms = 1000000000000UL / mask(capability, 63, 32);
  if (counter_clk_num_per_ms == 0) {
    OFFLOAD_FUNC(printf, "not enough resolution.\n");
    SHARED_SYMBOL(sync_flag) = 1;
    return;
  }

  int num_tim_cap = mask(capability, 12, 8);
  int i;
  // skip timer 0&1 for avoiding legacy routing.
  for (i = 2; i <= num_tim_cap; i++) {
    uint64_t timer_n_config = *((uint64_t *)(0xfed00000UL + 0x100 + i * 0x20));
    // check Tn_INT_ENB_CNF
    if (get_bit(timer_n_config, 2)) {
      // already enabled by Linux
      continue;
    }
    // check Tn_FSB_INT_DEL_CAP
    if (!get_bit(timer_n_config, 15)) {
      // does not support FSB interrupt delivery
      continue;
    }
    break;
  }
  if (i == num_tim_cap + 1) {
    OFFLOAD_FUNC(printf, "failed to find unused timer.\n");
    SHARED_SYMBOL(sync_flag) = 1;
    return;
  }
  OFFLOAD_FUNC(printf, "timer %d will be used.\n", i);

  uint64_t *main_counter_value_reg = (uint64_t *)(0xfed00000UL + 0xf0);
  uint64_t limit = *main_counter_value_reg + counter_clk_num_per_ms;

  *((uint64_t *)(0xfed00000UL + 0x108 + i * 0x20)) = limit;  // counter value
  *((uint64_t *)(0xfed00000UL + 0x110 + i * 0x20)) =
      vector | ((0xfee00000UL /* reserved */ | (1 << 12) /* destination */)
                << 32);  // fsb data & address
  asm volatile("sti");
  *((uint64_t *)(0xfed00000UL + 0x100 + i * 0x20)) =
      (1 << 14) | (1 << 2);  // Tn_FSB_EN_CNF | Tn_INT_ENB_CNF

  while (*main_counter_value_reg < limit + counter_clk_num_per_ms) {
    asm volatile("pause" ::: "memory");
  }
  SHARED_SYMBOL(sync_flag) = 1;
}
