#include "simple_loader/hakase.h"
#include "tests/test.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  if (argc < 2) {
    return 1;
  }

  auto file = SimpleLoader::BinaryFile::Load(argv[1]);
  if (!file) {
    return 1;
  }
  
  SimpleLoader sl(h2f, std::move(file));

  auto r = sl.Deploy();
  if (r.IsError()) {
    return 1;
  }
  r.Unwrap();

  Channel::Accessor<> ch_ac(h2f, 3);
  ch_ac.Write<uint64_t>(0, kDeployAddressStart);
  if (ch_ac.Do(1) != 0) {
    return 1;
  }
  int16_t type;
  f2h.WaitNewSignal(type);
  if (type != 1) {
    return 1;
  }

  return 0;
}
