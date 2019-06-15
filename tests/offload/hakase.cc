#include <toshokan/hakase/export.h>
#include <toshokan/hakase/hakase.h>
#include <unistd.h>
#include "shared.h"

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }

  SHARED_SYMBOL(sync_flag) = 0;
  SHARED_SYMBOL(state) = 0;

  boot(1);

  while (SHARED_SYMBOL(sync_flag) == 0) {
    offloader_tryreceive();
    asm volatile("pause" ::: "memory");
  }
  return (SHARED_SYMBOL(state) == 1);
}
