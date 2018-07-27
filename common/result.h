#pragma once
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "panic.h"

template<class T>
class Result {
public:
  Result() : _error(true) {
  }
  Result(T t) : _t(t), _error(false) {
  }
  ~Result() {
    if (_error && !_checked) {
      panic("Result: error: check the result\n");
    }
  }
  T Unwrap() {
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
