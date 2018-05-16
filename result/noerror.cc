#include "test.h"
#include "result.h"

int main(int argc, char **argv) {
  Result<bool> r(true);
  r.Unwrap();
  return 0;
}
