#include "common.h"
#include "app.h"

void show_i(F2H &f2h, int i) {
  char mes[] = "i=0";
  mes[2] += i;
  puts(f2h, mes);
}

int main() {
  F2H f2h;

  puts(f2h, "hello!");

  extern uint8_t _binary_app1_start[];
  extern uint8_t _binary_app1_size[];
  memcpy((void *)0x400000UL, (const void *)_binary_app1_start, (size_t)_binary_app1_size);
  
  Context c;
  c.next = (Context (*)(Context))0x400000UL; // entry1
  c.i = 0;
  
  c = c.next(c); // call entry1

  show_i(f2h, c.i);
  
  if (c.i != 1) {
    return_value(f2h, 1);
    return 1;
  }

  c = c.next(c); // call entry2

  show_i(f2h, c.i);

  if (c.i != 2) {
    return_value(f2h, 1);
    return 1;
  }

  puts(f2h, "bye!");

  return_value(f2h, 0);
  
  return 0;
}
