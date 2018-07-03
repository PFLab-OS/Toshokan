#include "elf_loader/hakase.h"
#include "tests/test.h"

int test_main(F2H &f2h, H2F &h2f, int argc, const char **argv) {
  if (argc < 2) {
    return 1;
  }

  auto file = ElfLoader::ElfFile::Load(argv[1]);
  if (!file) {
    return 1;
  }

  {
    auto r = file->Init(h2f);
    if (r.IsError()) {
      r.IgnoreError();
      return 1;
    }
    r.Unwrap();
  }
  
  ElfLoader sl(h2f, std::move(file));

  {
    auto r = sl.Deploy();
    if (r.IsError()) {
      r.IgnoreError();
      return 1;
    }
    r.Unwrap();
  }

  {
    auto r = sl.Execute(1);
    if (r.IsError()) {
      r.IgnoreError();
      return 1;
    }
    r.Unwrap();
  }

  int16_t type;
  f2h.WaitNewSignal(type);
  if (type != 1) {
    return 1;
  }

  return 0;
}
