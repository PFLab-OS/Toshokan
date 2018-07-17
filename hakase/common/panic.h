#pragma once

#if defined(__HAKASE__)

#define panic(x) assert(false)

#elif defined(__FRIEND__)

static inline void panic(const char *str=nullptr) {
  while(true) {
    __asm__ volatile("cli;hlt;");
  }
}

#endif /* defined(__HAKASE__) */
