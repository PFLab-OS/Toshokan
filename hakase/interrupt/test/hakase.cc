#include "interrupt/hakase.h"
#include "elf_loader/hakase.h"
#include "tests/test.h"
#include "common/_memory.h"

int test_main(F2H &f2h, H2F &h2f, I2H &i2h, int argc, const char **argv) {
  auto ic = InterruptController(i2h);

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

  for (int i = 0; i < 33; i++ ) { 
    switch (i) {
      case 8:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 17:
        continue;
    }

    int16_t type;
    i2h.WaitNewSignal(type);
    if (type != 6) {
      return 1;
    }

    int64_t vnum;
    bool p = ic.ProcessInterrupt(vnum);
    if (vnum != i) {
      return 1;
    }
  }

  return 0;
}
