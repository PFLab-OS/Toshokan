#pragma once

#if defined(__HAKASE__)

#include <assert.h>

#elif defined(__FRIEND__)

#include "common/panic.h"
#define assert(flag) if (!(flag)) { panic(); }

#endif /* defined(__HAKASE__) */
