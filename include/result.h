#pragma once
#include <assert.h>
#include <stdint.h>
#include <panic.h>

// TODO: need to contain error state
template <class T>
class Result {
 public:
  Result() : _error(true) {}
  Result(const T &t) : _t(t), _error(false) {}
  ~Result() noexcept(false) {
    if (_error && !_checked) {
      panic("Result: error: check the result\n");
    }
  }
  // Do not use substitution.
  // Use copy constructor instead.
  // MEMO: Since _error is const variable, it is difficult to implement copy
  // constructor. Result<T> &operator=(const Result<T> &);
  T &Unwrap() {
    if (_error) {
      panic("Result: error: failed to unwrap\n");
    }
    _checked = true;
    return _t;
  }
  bool IsError() {
    _checked = true;
    return _error;
  }

 private:
  T _t;
  const bool _error;
  bool _checked = false;
};
