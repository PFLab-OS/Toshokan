#include "../test.h"
#include "common/result.h"

int main(int argc, char **argv) {
  Result<bool> r(true);
  r.Unwrap(); // must Unwrap() when noerror
  return 0;
}
