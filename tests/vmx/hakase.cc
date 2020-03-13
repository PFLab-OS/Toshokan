#include <toshokan/hakase/hakase.h>
#include "shared.h"

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }
  SHARED_SYMBOL(is_vmx_available) = false;

  boot(1);

  while (!is_friend_stopped()) {
    asm volatile("pause" ::: "memory");
  }

  return SHARED_SYMBOL(is_vmx_available);
}
