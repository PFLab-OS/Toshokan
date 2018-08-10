#pragma once
#include <exception>

class AssertException : public std::exception {};

#define assert(x)  \
  if (!(x)) {      \
    assert_func(); \
  }

static inline void assert_func() { throw AssertException(); }
