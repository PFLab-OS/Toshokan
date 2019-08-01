#pragma once
#include <toshokan/_export.h>

#define EXPORT_SYMBOL(name) \
  __typeof__(&name) EXPORTED_SYMBOL(name) __attribute__((weak, used))
