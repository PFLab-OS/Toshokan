#include <time.h>
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

void panic_mock(const char*) { mock().actualCall("panic"); }

int main(int argc, char** argv) {
  srand((unsigned)time(NULL));
  return RUN_ALL_TESTS(argc, argv);
}
