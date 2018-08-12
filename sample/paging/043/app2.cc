#include "app.h"

extern "C" Context entry3(Context c) __attribute__((section(".text.entry")));
Context entry4(Context c);

Context entry3(Context c) {
  c.i = 0;
  c.next = entry4;
  return c;
}

Context entry4(Context c) {
  int *buf = (int *)0xC0001000;
  c.i = *buf;
  c.next = nullptr;
  return c;
}
