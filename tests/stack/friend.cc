#include <toshokan/friend/offload.h>
#include "shared.h"

uint64_t SHARED_SYMBOL(stack_addr)[kCoreCountMax];

uint64_t cnt = 0;

void friend_main() {
  int x = __sync_fetch_and_add(&cnt, 1);
  uint64_t rsp;
  
  asm volatile("mov %%rsp, %0" : "=g"(rsp));

  // 0x1000 == PAGE_SIZE
  // TODO implement stack size value in toshokan, and replace the const value with it
  SHARED_SYMBOL(stack_addr)[x] = (rsp & ~(0x1000 - 1)) + 0x1000;
}
