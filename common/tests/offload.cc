#include <pthread.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include "CppUTest/TestHarness.h"
#include <toshokan/offload.h>

TEST_GROUP(Offload){
  TEST_SETUP(){
    int ret = pthread_create(&th, NULL, &Offloader::Receiver, &c);
    if (ret) {
      err(EXIT_FAILURE, "could not create thread : %s", strerror(ret));
    }
  }

  TEST_TEARDOWN(){
    c.Stop();
    int ret = pthread_join(th, NULL);
    if (ret) {
      err(EXIT_FAILURE, "could not join thread : %s", strerror(ret));
    }
  }
  pthread_t th;

  Offloader c;

  void test_func(int *i) {
    (*i)++;
  }
};

TEST(Offload, MultipleExecution) {
  int var = 0;
  int n = rand() % 20 + 10;
  for (int i = 0; i < n; i++) {
    OFFLOAD(c, {var++;});
  }
  CHECK_EQUAL(n, var);
}

TEST(Offload, ComplexMultipleExecution) {
  int var = 0;
  int n = rand() % 20 + 10;
  int m = 0;
  int o = 0;
  for(int i = 0; i < n; i++) {
    int r = rand() % 2;
    if (r) {
      o++;
    } else {
      o--;
    }
    OFFLOAD(c, 
	    if (r) {
	      m++;
	    } else {
	      m--;
	    }
	    );
  }
  CHECK_EQUAL(o, m);
}

TEST(Offload, MultipleExecutionWithFunctionCall) {
  int var = 0;
  int n = rand() % 20 + 10;
  for (int i = 0; i < n; i++) {
    OFFLOAD(c, { test_func(&var); });
  }
  CHECK_EQUAL(n, var);
}
