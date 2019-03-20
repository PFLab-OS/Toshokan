#include <stdint.h>
#include "_memory.h"
#include "channel_accessor2.h"

int main() __attribute__((section(".text.boot")));

int main() {
  F2H2 f2h;

  CallerChannelAccessor caller_ca(f2h, Channel2::Id(0),
                                  Channel2::Signal::kCallback());
  caller_ca.Call();
  return 0;
}
