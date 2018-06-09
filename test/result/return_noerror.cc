#include "../test.h"
#include "common/result.h"

Result<bool> func() {
  return Result<bool>(true);
}

int main(int argc, char **argv) {
  Result<bool> r = func();
  if (r.IsError()) {
    return 1;
  }
  r.Unwrap(); // must Unwrap() when noerror
  return 0;
}
