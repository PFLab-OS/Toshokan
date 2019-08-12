#pragma once
#include <toshokan/_export.h>

#define EXPORT_SYMBOL(name)                                     \
  extern __typeof__(&name) EXPORTED_SYMBOL(name);               \
  ExportSymbolContainer export_symbol_##name                    \
      __attribute__((weak, used, section("export_symbol"))) = { \
          (void **)&EXPORTED_SYMBOL(name), (void *)&name}
