#pragma once

#ifdef __FRIEND__
static inline void panic(const char *str = nullptr) {
  while (true) {
    __asm__ volatile("cli;hlt;");
  }
}
#endif /* __FRIEND__ */

#ifdef __HAKASE__
#include <assert.h>

static inline void panic(const char *str = nullptr) { assert(false); }
#endif /* __HAKASE__ */
