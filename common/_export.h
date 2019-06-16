#pragma once
#include <toshokan/symbol.h>

struct ExportSymbolContainer {
  void **shared_var_ptr;
  void *hakase_var_ptr;
};

#define EXPORTED_SYMBOL(name) SHARED_SYMBOL(import_symbol_##name)
