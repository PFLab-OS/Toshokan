#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "../result.h"

TEST_GROUP(Result){TEST_SETUP(){}

                   TEST_TEARDOWN(){mock().clear();
}
}
;

TEST(Result, Error) {
  mock().expectNoCall("panic");
  {
    Result<bool> r;
    CHECK(r.IsError());
  }
  mock().checkExpectations();
}

TEST(Result, NoError) {
  mock().expectNoCall("panic");
  {
    Result<bool> r(true);
    CHECK_FALSE(r.IsError());
  }
  mock().checkExpectations();
}

// must check result
TEST(Result, IgnoreWhenError) {
  mock().expectNoCall("panic");
  {
    Result<bool> r;
    r.IsError();
  }
  mock().checkExpectations();
}

TEST(Result, AbleToUnwrapWhenNoError) {
  mock().expectNoCall("panic");
  {
    Result<bool> r(true);
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
    r.Unwrap();
  }
  mock().checkExpectations();
}

// must check result
TEST(Result, NotcheckedWhenError) {
  mock().expectOneCall("panic");
  { Result<bool> r; }
  mock().checkExpectations();
}
