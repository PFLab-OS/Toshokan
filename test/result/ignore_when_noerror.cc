#include "../test.h"
#include "common/result.h"

// must be crashed
int main(int argc, char **argv) {
  Result<bool> r(true);
  r.IgnoreError(); // do not IgnoreError() when noerror
  return 0;
}
