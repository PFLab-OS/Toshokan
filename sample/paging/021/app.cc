#include "app.h"

extern "C" Context entry1(Context c) __attribute__ ((section (".text.entry")));
Context entry2(Context c);

Context entry1(Context c) {
  c.i = 1;
  c.next = entry2;
  return c;
}

Context entry2(Context c) {
  c.i = 2;
  c.next = nullptr;
  return c;
}

