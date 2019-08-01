#include <toshokan/hakase/hakase.h>
#include "shared.h"

int state;

int func(int i, int j) {
  state = i - j;
  return 100;
}

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }

  state = 0;

  boot(1);

  while (!is_friend_stopped()) {
    offloader_tryreceive();
    asm volatile("pause" ::: "memory");
  }

  return (state == 1) && (SHARED_SYMBOL(sync_flag) == 1);
}
