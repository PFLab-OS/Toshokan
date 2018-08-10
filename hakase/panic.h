#pragma once
#include <assert.h>

static inline void panic(const char *str = nullptr) { assert(false); }
