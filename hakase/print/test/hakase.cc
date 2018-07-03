#include "tests/test.h"
#include "common/channel.h"
#include "print/hakase.h"
#include <iostream>

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  h2f.Reserve(1);
  h2f.SendSignal(2);
  h2f.Release();

  StringReceiver sr(f2h);
  sr.Do();
  
  auto str = sr.GetString();
  if (!str || *str != "abc\n") {
    return 1;
  }
  
  return 0;
}
