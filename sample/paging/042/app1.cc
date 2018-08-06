#include "app.h"

extern "C" Context entry1(Context c) __attribute__((section(".text.entry")));
Context entry2(Context c);

Context entry1(Context c) {
  c.i = 7;
  c.next = entry2;
  return c;
}

Context entry2(Context c) {
  c.i++;
  int *buf = (int *)0xC0001000;
  *buf = c.i;
  c.next = nullptr;
  return c;
}
