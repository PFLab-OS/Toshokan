#include "simple_loader/hakase.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  if (argc < 2) {
    return 1;
  }

  auto file = SimpleLoader::BinaryFile::Load(argv[1]);
  if (!file) {
    return 1;
  }
  
  SimpleLoader sl(h2f, std::move(file));
  
  if (sl.Deploy() != 0) {
    return 1;
  }

  h2f.Reserve();
  h2f.Write(0, kDeployAddressStart);
  if (h2f.SendSignal(3) != 0) {
    return 1;
  }
  if (f2h.WaitNewSignal() != 1) {
    return 1;
  }

  return 0;
}
