#pragma once

#include "panic.h"
#define assert(flag) \
  if (!(flag)) {     \
    panic();         \
  }
