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
    if (!_checked) {
      panic("Result: error: check the result\n");
    }
  }
  T Unwrap() {
    _checked = true;
    if (_error) {
      panic("Result: error: failed to unwrap\n");
    }
    return _t;
  }
  bool IsError() {
    return _error;
  }
  void IgnoreError() {
    if (!_error) {
      panic("Result: error: do not call IgnoreError() when there is no error.\n");
    }
    _checked = true;
  }
private:
  T _t;
  const bool _error;
  bool _checked = false;
};
