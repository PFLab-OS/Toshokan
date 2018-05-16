#include "test.h"
#include "result.h"

int main(int argc, char **argv) {
  Result<bool> r;
  if (!r.IsError()) {
    return 1;
  }
  r.IgnoreError();
  return 0;
}
