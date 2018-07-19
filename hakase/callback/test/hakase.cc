#include "tests/test.h"
#include "channel/hakase.h"
#include "common/channel_accessor.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  int16_t id = 1;
  ChannelAccessor<> ch_ac(h2f, id);
  ch_ac.Do(1);

  int16_t type;
  if (f2h.WaitNewSignal(type) != id) {
    return 1;
  }
  if (type != 1) {
    return 1;
  }
  return 0;
}
