#pragma once
#include <stdlib.h>
#include <assert.h>

template<class T>
class Result {
public:
  Result() : _error(true) {
  }
  Result(T t) : _t(t), _error(false) {
  }
  ~Result() {
    if (!_checked) {
      fprintf(stderr, "Result: error: check the result\n");
      exit(255);
    }
  }
  T Unwrap() {
    _checked = true;
    if (_error) {
      fprintf(stderr, "Result: error: failed to unwrap\n");
      exit(255);
    }
    return _t;
  }
  bool IsError() {
    return _error;
  }
  void IgnoreError() {
    if (!_error) {
      fprintf(stderr, "Result: error: do not call IgnoreError() when there is no error.\n");
      exit(255);
    }
    _checked = true;
  }
private:
  T _t;
  const bool _error;
  bool _checked = false;
};
