#include "test.h"
#include "result.h"

int main(int argc, char **argv) {
  Result<bool> r;
  r.Unwrap();
  return 0;
}
