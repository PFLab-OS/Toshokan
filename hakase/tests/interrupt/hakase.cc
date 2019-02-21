#include "common/_memory.h"
#include "elf_loader.h"
#include "interrupt.h"
#include "tests/test.h"

int test_main(F2H &f2h, H2F &h2f, I2H &i2h, int argc, const char **argv) {
  auto ic = InterruptController(i2h);
  ic.Init();

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

  for (int i = 0; i < 33; i++) {
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
