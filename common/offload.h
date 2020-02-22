#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef size_t toshokan_jmp_buf[10];

int toshokan_setjmp(toshokan_jmp_buf env);
int toshokan_setjmp_with_wait(toshokan_jmp_buf env);
int toshokan_longjmp(toshokan_jmp_buf env, int val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

class Offloader {
 public:
  bool TryReceive() {
    if (_friend_ctx[8] == 0) {
      return false;
    }
    _friend_ctx[8] = 0;
    if (toshokan_setjmp(_hakase_ctx) == 0) {
      toshokan_longjmp(_friend_ctx, 1);
    } else {
      _friend_ctx[9] = 1;
    }
    return true;
  }

  toshokan_jmp_buf _friend_ctx, _hakase_ctx;

  int _state1 = 0;
  int _state2 = 0;
  int _lock = 0;

 private:
};

#define _OFFLOAD(c, code)                                     \
  do {                                                        \
    while ((c)._lock != 0 ||                                  \
           !__sync_bool_compare_and_swap(&(c)._lock, 0, 1)) { \
      asm volatile("pause" ::: "memory");                     \
    }                                                         \
    if (toshokan_setjmp_with_wait((c)._friend_ctx) != 0) { \
      asm volatile("" ::: "memory");                          \
      code;                                                   \
      asm volatile("" ::: "memory");			      \
      if (toshokan_setjmp((c)._friend_ctx) == 0) {            \
         toshokan_longjmp((c)._hakase_ctx, 1);		      \
      }							      \
    }                                                         \
    (c)._lock = 0;                                            \
  } while (0);
