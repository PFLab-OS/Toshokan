#include "test.h"
#include "result.h"

// must be crashed
int main(int argc, char **argv) {
  Result<bool> r;
  r.Unwrap(); // do not Unwrap() when error
  return 0;
}
