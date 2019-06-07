#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef size_t toshokan_jmp_buf[8];

int toshokan_setjmp(toshokan_jmp_buf env);
int toshokan_longjmp(toshokan_jmp_buf env, int val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

class Offloader {
 public:
  void Stop() { _stop = 1; }
  static void *Receiver(void *arg) {
    Offloader *c = reinterpret_cast<Offloader *>(arg);
    while (!c->_stop) {
      if (c->_state1 == 0) {
        continue;
      }
      c->_state1 = 0;
      if (toshokan_setjmp(c->_buf2) == 0) {
        toshokan_longjmp(c->_buf1, 1);
      } else {
        c->_state2 = 1;
      }
      asm volatile("" ::: "memory");
    }
    return NULL;
  }

  toshokan_jmp_buf _buf1, _buf2;

  int _state1 = 0;
  int _state2 = 0;

 private:
  int _stop = 0;
};

#define OFFLOAD(c, code)                 \
  do {                                   \
    if (toshokan_setjmp(c._buf1) == 0) { \
      c._state1 = 1;                     \
      while (c._state2 == 0) {           \
        asm volatile("" ::: "memory");   \
      }                                  \
      c._state2 = 0;                     \
    } else {                             \
      code toshokan_longjmp(c._buf2, 1); \
    }                                    \
  } while (0);
