#include <stdio.h>
#include <toshokan/hakase/export.h>
#include <toshokan/hakase/hakase.h>
#include <unistd.h>
#include "shared.h"

EXPORT_SYMBOL(printf);

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }

  SHARED_SYMBOL(sync_flag) = 0;

  printf("starting up friend cores...");
  fflush(stdout);
  int cpunum = boot(0);
  printf("done\n");
  fflush(stdout);

  while (SHARED_SYMBOL(sync_flag) != cpunum) {
    offloader_tryreceive();
    usleep(1000);
  }
  return 1;
}

int main(int argc, const char **argv) {
  if (test_main() > 0) {
    printf("\e[32m%s: PASSED\e[m\n", argv[0]);
    return 0;
  } else {
    printf("\e[31m%s: FAILED\e[m\n", argv[0]);
    return 255;
  }
}
