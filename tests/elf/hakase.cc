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

  int cpunum = boot(0);

  sleep(1);

  return (SHARED_SYMBOL(sync_flag) == cpunum);
}
