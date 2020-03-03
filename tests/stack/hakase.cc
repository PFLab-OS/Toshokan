#include <toshokan/hakase/hakase.h>
#include "shared.h"
#include <stdlib.h>
#include <stdio.h>

int asc(const void *a, const void *b) {
  uint64_t *A = (uint64_t *)a;
  uint64_t *B = (uint64_t *)b;
  if (*A > *B) return 1;
  if (*A < *B) return -1;
  return 0;
}

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }

  for (int i = 0; i < 0x10; i++) {
    SHARED_SYMBOL(stack_addr)[i] = (uint64_t)~0x0;
  }

  int cpunum = boot(0);

  if (cpunum > 0x10) {
    printf("Too many cpunum\n");
    return -1;
  }

  while (!is_friend_stopped()) {
    asm volatile("pause" ::: "memory");
  }

  for (int i = 0; i < 0x10; i++) {
    if (!((i < cpunum && SHARED_SYMBOL(stack_addr)[i] != 0) || (i >= cpunum && SHARED_SYMBOL(stack_addr)[i] == (uint64_t)~0x0))) {
      printf("Invalid entry\n");
      return -1;
    }
  }
  //sort
  qsort(SHARED_SYMBOL(stack_addr), cpunum, sizeof(uint64_t), asc);

  uint64_t pre = SHARED_SYMBOL(stack_addr)[0];
  for (int i = 1; i < cpunum; i++) {
    if (pre == SHARED_SYMBOL(stack_addr)[i]) return -1;
    pre = SHARED_SYMBOL(stack_addr)[i];
  }

  return 1;
}
