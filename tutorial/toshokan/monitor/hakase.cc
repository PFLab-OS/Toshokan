#include <stdio.h>
#include <toshokan/hakase/hakase.h>
#include <unistd.h>

int test_main() {
  int r;
  r = setup();
  if (r != 0) {
    return r;
  }

  boot(0);

  while (true) {
    usleep(1000 * 1000);
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
