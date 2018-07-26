#include <stdio.h>
#include <stdlib.h>
#include "CppUTest/TestHarness.h"

TEST_GROUP(Ivshmem) {
  TEST_SETUP() {
  }

  TEST_TEARDOWN() {
  }
};

 TEST(Ivshmem, CheckIfMemorybackendfileExists) {
  system("/root/qemu");
  FILE *fp = fopen("/dev/shm/channel", "rb+");
  CHECK(fp != NULL);
  fclose(fp);
}
