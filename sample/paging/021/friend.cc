#include "common.h"
#include "app.h"

int main() {
  F2H f2h;

  puts(f2h, "hello!");

  extern uint8_t _binary_app_start[];
  extern uint8_t _binary_app_size[];
  memcpy((void *)0x400000UL, (const void *)_binary_app_start, (size_t)_binary_app_size);
  
  Context c;
  c.next = (Context (*)(Context))0x400000UL; // entry1
  c.i = 0;
  
  c = c.next(c); // call entry1

  if (c.i != 1) {
    return_value(f2h, 1);
    return 1;
  }

  c = c.next(c); // call entry2

  if (c.i != 2) {
    return_value(f2h, 1);
    return 1;
  }

  puts(f2h, "bye!");

  return_value(f2h, 0);
  
  return 0;
}
