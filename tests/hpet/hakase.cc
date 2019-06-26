#include <stddef.h>
#include <stdio.h>
#include <toshokan/hakase/export.h>
#include <toshokan/hakase/hakase.h>
#include "shared.h"

int state;

void func(int i, int j) { state = i - j; }

EXPORT_SYMBOL(func);
EXPORT_SYMBOL(printf);

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

extern Page SHARED_SYMBOL(pml4t);
extern Page SHARED_SYMBOL(pdpt);
extern Page SHARED_SYMBOL(pd);

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }

  static const size_t k256TB = 256UL * 1024 * 1024 * 1024 * 1024;
  static const size_t k512GB = 512UL * 1024 * 1024 * 1024;
  static const size_t k1GB = 1024UL * 1024 * 1024;
  static const size_t k2MB = 2UL * 1024 * 1024;
  SHARED_SYMBOL(pdpt).entry[(0xC0000000 % k512GB) / k1GB] =
      0xC0000000 | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 7);

  SHARED_SYMBOL(sync_flag) = 0;
  state = 0;

  boot(1);

  while (SHARED_SYMBOL(sync_flag) == 0) {
    offloader_tryreceive();
    asm volatile("pause" ::: "memory");
  }

  return (state == 1);
}
