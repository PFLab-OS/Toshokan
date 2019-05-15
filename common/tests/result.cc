#include <toshokan/result.h>
#include "CppUTest/TestHarness.h"

TEST_GROUP(Result){TEST_SETUP(){}

                   TEST_TEARDOWN(){}};

TEST(Result, Error) {
  Result<bool> r;
  CHECK(r.IsError());
}

TEST(Result, NoError) {
  Result<bool> r(true);
  CHECK_FALSE(r.IsError());
}

// must check result
TEST(Result, IgnoreWhenError) {
  Result<bool> r;
  r.IsError();
}

TEST(Result, AbleToUnwrapWhenNoError) {
  Result<bool> r(true);
  r.Unwrap();
}

//
// Do not write following patterns
//

// do not Unwrap() when error
TEST(Result, UnwrapWhenError) {
  Result<bool> r;
  CHECK_THROWS(PanicException, r.Unwrap());
  r.IsError();
}

// must check result
TEST(Result, NotcheckedWhenError) {
  CHECK_THROWS(PanicException, ([]() { Result<bool> r; })());
}
