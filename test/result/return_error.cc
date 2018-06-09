#include "../test.h"
#include "common/result.h"

Result<bool> func() {
  return Result<bool>();
}

int main(int argc, char **argv) {
  Result<bool> r = func();
  if (!r.IsError()) {
    return 1;
  }
  // do error handling
  r.IgnoreError(); // must IgnoreError() after you finished error handling
  return 0;
}
