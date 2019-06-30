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

  boot(0);

  while(!is_friend_stopped()) {
    asm volatile("pause":::"memory");
  }

  return (SHARED_SYMBOL(notify) == &hakase_var);
}
