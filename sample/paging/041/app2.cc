#include "app.h"

extern "C" Context entry3(Context c) __attribute__((section(".text.entry")));
Context entry4(Context c);

Context entry3(Context c) {
  c.i = 9;
  c.next = entry4;
  return c;
}

Context entry4(Context c) {
  c.i--;
  c.next = nullptr;
  return c;
}
