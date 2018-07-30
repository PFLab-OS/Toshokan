#include "CppUTest/CommandLineTestRunner.h"
#include <time.h>

int main(int argc, char** argv) {
  srand((unsigned)time(NULL));
  return RUN_ALL_TESTS(argc, argv);
}
