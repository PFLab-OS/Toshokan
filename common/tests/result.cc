#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#define panic(x) panic_mock(x)
void panic_mock(const char *);

#include "../result.h"

TEST_GROUP(Result) {
  TEST_SETUP() {
  }

  TEST_TEARDOWN() {
    mock().clear();
  }
};


// must IgnoreError() after you finished error handling
TEST(Result, IgnoreWhenError) {
  mock().expectNoCall("panic");
  {
    Result<bool> r;
    CHECK(r.IsError());
    r.IgnoreError();
  }
  mock().checkExpectations();
}

// must Unwrap() when noerror
TEST(Result, UnwrapWhenNoError) {
  mock().expectNoCall("panic");
  {
    Result<bool> r(true);
    CHECK_FALSE(r.IsError());
    r.Unwrap();
  }
  mock().checkExpectations();
}

//
// Do not write following patterns
//

// do not Unwrap() when error
TEST(Result, UnwrapWhenError) {
  mock().expectOneCall("panic");
  {
    Result<bool> r;
    CHECK(r.IsError());
    r.Unwrap();
  }
  mock().checkExpectations();
}

// must check result
TEST(Result, NotcheckedWhenNoError) {
  mock().expectOneCall("panic");
  {
    Result<bool> r(true);
    CHECK_FALSE(r.IsError());
  }
  mock().checkExpectations();
}

// must check result
TEST(Result, NotcheckedWhenError) {
  mock().expectOneCall("panic");
  {
    Result<bool> r;
    CHECK(r.IsError());
  }
  mock().checkExpectations();
}

// do not IgnoreError() when noerror
TEST(Result, IgnoreWhenNoErrror) {
  mock().expectOneCall("panic");
  {
    Result<bool> r(true);
    CHECK_FALSE(r.IsError());
    r.IgnoreError();
  }
  mock().checkExpectations();
}

