#include <string.h>
#include "test.h"
#include "common/channel.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  static const uint32_t kTestVectorNum = 32;
  h2f.Reserve();

  h2f.Write(0, kTestVectorNum);

  h2f.SendSignal(5);


  if (f2h.WaitNewSignal() != 5) {
    return 1;
  }

  uint32_t data;
  f2h.Read(8, data);
  f2h.Return(0);

  if (data != 32) {
    return 1;
  }

  return 0;
}
