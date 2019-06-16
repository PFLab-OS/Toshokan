#pragma once
#include <exception>

class PanicException : public std::exception {};

#define panic(x) panic_func(x)
static inline void panic_func(const char *str = nullptr) {
  throw PanicException();
}
