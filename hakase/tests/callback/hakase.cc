#include <stdio.h>
#include "channel2.h"
#include "channel_accessor2.h"
#include "tests/test.h"

int test_main(F2H2 &f2h, H2F2 &h2f, I2H2 &i2h, int argc, const char **argv) {
  Channel2::Id id(1);
  CallerChannelAccessor caller_ca(h2f, id, Channel2::Signal::kCallback());
  caller_ca.Call();

  CalleeChannelAccessor callee_ca(f2h);
  callee_ca.ReceiveSignal();
  if (callee_ca.GetSignal() != Channel2::Signal::kCallback()) {
    return 1;
  }
  return 0;
}
