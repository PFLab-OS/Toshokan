#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <toshokan/offload.h>
#include "CppUTest/TestHarness.h"

TEST_GROUP(Offload) {
  TEST_SETUP() {
    int ret = pthread_create(&th, NULL, &receiver_thread, &c);
    if (ret) {
      err(EXIT_FAILURE, "could not create thread : %s", strerror(ret));
    }
  }

  TEST_TEARDOWN() {
    c.stop = 1;
    int ret = pthread_join(th, NULL);
    if (ret) {
      err(EXIT_FAILURE, "could not join thread : %s", strerror(ret));
    }
  }
  pthread_t th;

  struct Container {
    Offloader offloader;
    int stop;
  } c;

  void test_func(int *i) { (*i)++; }
  static void *receiver_thread(void *arg) {
    Container *c = reinterpret_cast<Container *>(arg);
    while (!c->stop) {
      c->offloader.TryReceive();
      asm volatile("pause" ::: "memory");
    }
    return NULL;
  }
};

TEST(Offload, MultipleExecution) {
  int var = 0;
  int n = rand() % 20 + 10;
  for (int i = 0; i < n; i++) {
    OFFLOAD(c.offloader, { var++; });
  }
  CHECK_EQUAL(n, var);
}

TEST(Offload, ComplexMultipleExecution) {
  int var = 0;
  int n = rand() % 20 + 10;
  int m = 0;
  int o = 0;
  for (int i = 0; i < n; i++) {
    int r = rand() % 2;
    if (r) {
      o++;
    } else {
      o--;
    }
    OFFLOAD(c.offloader, if (r) { m++; } else { m--; });
  }
  CHECK_EQUAL(o, m);
}

TEST(Offload, MultipleExecutionWithFunctionCall) {
  int var = 0;
  int n = rand() % 20 + 10;
  for (int i = 0; i < n; i++) {
    OFFLOAD(c.offloader, { test_func(&var); });
  }
  CHECK_EQUAL(n, var);
}
