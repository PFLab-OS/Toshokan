#include "../test.h"
#include "result.h"

// must be crashed
int main(int argc, char **argv) {
  Result<bool> r(true);
  // check result
  return 0;
}
