#pragma once
#include <toshokan/_export.h>

#define EXPORT_SYMBOL(name)                                 \
  extern void *EXPORTED_SYMBOL(name);                       \
  ExportSymbolContainer export_symbol_##name __attribute__( \
      (section("export_symbol"))) = {&EXPORTED_SYMBOL(name), (void *)&name}
