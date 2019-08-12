#include <toshokan/hakase/hakase.h>
#include <unistd.h>
#include "shared.h"

int func() { return 0; }

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }

  boot(0);

  while (!is_friend_stopped()) {
    asm volatile("pause" ::: "memory");
  }

  return (SHARED_SYMBOL(notify) == &func);
}
