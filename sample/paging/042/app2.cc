#include "app.h"

extern "C" Context entry3(Context c) __attribute__((section(".text.entry")));
Context entry4(Context c);

Context entry3(Context c) {
  c.i = 9;
  c.next = entry4;
  return c;
}

Context entry4(Context c) {
  // invokes the system call which maps physical memory
  char *buf = (char *)0xC0001000;
  for (int i = 0; i < 10; i++) {
    buf[i] = i;
  }
  c.i--;
  c.next = nullptr;
  return c;
}
