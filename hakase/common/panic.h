#pragma once

#if defined(__HAKASE__)

#define panic() assert(false)

#elif defined(__FRIEND__)

static inline void panic() {
  while(true) {
    __asm__ volatile("cli;hlt;");
  }
}

#endif /* defined(__HAKASE__) */
