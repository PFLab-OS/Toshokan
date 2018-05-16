#pragma once
#include <stdio.h>

static inline void show_result(bool result, const char *msg = nullptr) {
  printf("test result: %s", result ? "\x1b[34mOK\x1b[39m" : "\x1b[31mFAILED\x1b[39m");
  if (!result && msg != nullptr) {
    printf("reason: %s", msg);
  }
  printf("\n");
}
