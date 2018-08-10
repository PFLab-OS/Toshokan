#include <stdlib.h>
#include <time.h>
#include <new>
#include "CppUTest/CommandLineTestRunner.h"

int main(int argc, char** argv) {
  srand((unsigned)time(NULL));
  return RUN_ALL_TESTS(argc, argv);
}
