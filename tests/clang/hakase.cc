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

  int cpunum = boot(0);

  while(!is_friend_stopped()) {
    asm volatile("pause":::"memory");
  }

  return (SHARED_SYMBOL(sync_flag) == cpunum);
}
