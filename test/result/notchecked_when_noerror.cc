#include "../test.h"
#include "common/result.h"

// must be crashed
int main(int argc, char **argv) {
  Result<bool> r(true);
  // check result
  return 0;
}
