#pragma once
#include <toshokan/_export.h>

template <class, class>
struct is_compatible {
  enum { value = 0 };
};
template <class T>
struct is_compatible<T, T> {
  enum { value = 1 };
};

#define EXPORT_SYMBOL(name)                                      \
  static_assert(is_compatible<__typeof__(EXPORTED_SYMBOL(name)), \
                              __typeof__(&name)>::value,         \
                "type does not match between '" #name            \
                "' and 'EXPORTED_SYMBOL(" #name ")' .");         \
  ExportSymbolContainer export_symbol_##name                     \
      __attribute__((section("export_symbol"))) = {              \
          (void **)&EXPORTED_SYMBOL(name), (void *)&name}
