#pragma once
#include <assert.h>
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <new>
#include "panic.h"

template <class T>
class Result {
public:
  Result() : _t(nullptr), _error(true) {
  }
  Result(const T &t) : _t(reinterpret_cast<T *>(_buf)), _error(false) {
    new (_buf) T(t);
  }
  ~Result() noexcept(false) {
    if (_error && !_checked) {
      panic("Result: error: check the result\n");
    }
    if (!_error) {
      _t->~T();
    }
  }
  // Do not use substitution.
  // Use copy constructor instead.
  // Result<T> &operator=(const Result<T> &);
  T Unwrap() {
    if (_error) {
      panic("Result: error: failed to unwrap\n");
    }
    _checked = true;
    return *_t;
  }
  bool IsError() {
    _checked = true;
    return _error;
  }
private:
  uint8_t _buf[sizeof(T)];
  T *_t;
  const bool _error;
  bool _checked = false;
};
