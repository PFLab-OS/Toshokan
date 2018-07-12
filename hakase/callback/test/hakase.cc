#include "tests/test.h"
#include "common/channel.h"

int test_main(F2H &f2h, H2F &h2f, I2H &i2h, int argc, const char **argv) {
  int16_t id = 1;
  Channel::Accessor ch_ac(h2f, id);
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
