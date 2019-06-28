#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <toshokan/offload.h>
#include "CppUTest/TestHarness.h"

struct Container {
  Offloader offloader;
  int stop;
};

void test_func(int *i) { (*i)++; }

TEST_GROUP(Offload) {
  Container c;
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
    _OFFLOAD(c.offloader, { var++; });
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
    _OFFLOAD(c.offloader, if (r) { m++; } else { m--; });
  }
  CHECK_EQUAL(o, m);
}

TEST(Offload, MultipleExecutionWithFunctionCall) {
  int var = 0;
  int n = rand() % 20 + 10;
  for (int i = 0; i < n; i++) {
    _OFFLOAD(c.offloader, { test_func(&var); });
  }
  CHECK_EQUAL(n, var);
}

struct TestContainer {
  int i;
  Container *c;
};

int x = 0;

void *MultipleThreadSubFunc(void *arg) {
  TestContainer *tc = (TestContainer *)arg;
  int n = rand() % 20 + 10;
  for (int i = 0; i < n; i++) {
    _OFFLOAD(tc->c->offloader, { test_func(&tc->i); });
  }
  tc->i -= n;
  return NULL;
}

TEST(Offload, MultipleThread) {
  TestContainer tc[10];
  pthread_t th[10];
  for (int i = 0; i < 10; i++) {
    tc[i].i = 0;
    tc[i].c = &c;
    pthread_create(&th[i], NULL, &MultipleThreadSubFunc, &tc[i]);
  }
  int var_sum = 0;
  for (int i = 0; i < 10; i++) {
    pthread_join(th[i], NULL);
    var_sum += tc[i].i;
  }
  CHECK_EQUAL(0, var_sum);
}
