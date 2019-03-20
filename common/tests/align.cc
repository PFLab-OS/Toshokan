#include "align.h"
#include "CppUTest/TestHarness.h"

TEST_GROUP(Align){TEST_SETUP(){}

                  TEST_TEARDOWN(){}};

TEST(Align, Check1) { CHECK_EQUAL(20, align<uint8_t>(20, 5)); }

TEST(Align, Check2) { CHECK_EQUAL(100, align<uint8_t>(103, 100)); }

TEST(Align, Check3) { CHECK_EQUAL(0, align<uint8_t>(3, 6)); }

TEST(Align, CheckSizet) {
  CHECK_EQUAL(0x200000000, align<size_t>(0x2FFFFFFFF, 0x100000000));
}
