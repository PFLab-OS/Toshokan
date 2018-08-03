#include "elf_loader/hakase.h"
#include "tests/test.h"

int test_main(F2H &f2h, H2F &h2f, I2H &i2h, int argc, const char **argv) {
  if (argc < 2) {
    return 1;
  }

  auto file = ElfLoader::ElfFile::Load(argv[1]);
  if (!file) {
    return 1;
  }

  if (file->Init(h2f).IsError()) {
    return 1;
  }

  ElfLoader sl(h2f, std::move(file));

  if (sl.Deploy().IsError()) {
    return 1;
  }

  if (sl.Execute(1).IsError()) {
    return 1;
  }

  int16_t type;
  f2h.WaitNewSignal(type);
  if (type != 1) {
    return 1;
  }

  return 0;
}
