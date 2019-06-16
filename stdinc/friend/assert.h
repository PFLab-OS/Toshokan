#pragma once

#include <toshokan/panic.h>
#define assert(flag) \
  if (!(flag)) {     \
    panic();         \
  }
