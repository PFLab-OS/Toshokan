#include "app.h"

extern "C" Context entry1(Context c) __attribute__((section(".text.entry")));
Context entry2(Context c);

Context entry1(Context c) {
  // invokes the system call which maps physical memory
  char *buf = (char *)0xC0001000;
  for (int i = 0; i < 10; i++) {
    buf[i] = i;
  }
  c.i = 1;
  c.next = entry2;
  return c;
}

Context entry2(Context c) {
  c.i++;
  c.next = nullptr;
  return c;
}
