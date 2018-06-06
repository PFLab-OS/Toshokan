#include "test.h"
#include "channel.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  h2f.Reserve();
  h2f.SendSignal(1);

  if (f2h.WaitNewSignal() != 1) {
    return 1;
  }
  return 0;
}
