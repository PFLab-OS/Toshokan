#include <toshokan/hakase/hakase.h>
#include <unistd.h>
#include "shared.h"

int hakase_var;
EXPORT_SYMBOL(hakase_var);

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }

  SHARED_SYMBOL(sync_flag) = 0;

  int cpunum = boot(0);

  sleep(1);

  return (SHARED_SYMBOL(sync_flag) == cpunum) &&
         (SHARED_SYMBOL(notify) == &hakase_var);
}
