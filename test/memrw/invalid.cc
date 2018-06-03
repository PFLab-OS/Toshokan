#include <assert.h>
#include "../test.h"
#include "channel.h"

int test_main(F2H &f2h, H2F &h2f) {
  static const uint32_t kInvalid = 2;
  h2f.Reserve();
  h2f.Write(0, kInvalid);
  assert(h2f.SendSignal(4) < 0);
  return 0;
}
