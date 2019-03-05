#include "channel_accessor2.h"
#include "simple_loader.h"
#include "tests/test.h"

int test_main(F2H2 &f2h, H2F2 &h2f, I2H2 &i2h, int argc, const char **argv) {
  if (argc < 2) {
    return 1;
  }

  auto file = SimpleLoader::BinaryFile::Load(argv[1]);
  if (!file) {
    return 1;
  }

  SimpleLoader sl(h2f, std::move(file));

  if (sl.Deploy().IsError()) {
    return 1;
  }

  CallerChannelAccessor caller_ca(h2f, Channel2::Id(1),
                                  Channel2::Signal::kExec());
  caller_ca.Write<uint64_t>(CallerChannelAccessor::Offset<uint64_t>(0),
                            kDeployAddressStart);
  if (caller_ca.Call() != 0) {
    return 1;
  }

  CalleeChannelAccessor callee_ca(f2h);
  callee_ca.ReceiveSignal();
  if (callee_ca.GetSignal() != Channel2::Signal::kCallback()) {
    return 1;
  }
  callee_ca.Return(0);

  return 0;
}
