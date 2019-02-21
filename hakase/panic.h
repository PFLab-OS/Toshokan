#pragma once
#include <assert.h>

#ifdef __HAKASE__
static inline void panic(const char *str = nullptr) { assert(false); }
#endif /* __HAKASE__ */
