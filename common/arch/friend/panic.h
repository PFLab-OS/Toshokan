#pragma once

static inline void panic(const char *str = nullptr) {
  while (true) {
    __asm__ volatile("cli;hlt;");
  }
}
