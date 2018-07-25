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
  extern uint8_t _binary_app2_start[];
  extern uint8_t _binary_app2_size[];
  memcpy((void *)0x400000UL, (const void *)_binary_app1_start, (size_t)_binary_app1_size);
  memcpy((void *)0x500000UL, (const void *)_binary_app2_start, (size_t)_binary_app2_size);
  
  Context c1;
  c1.next = (Context (*)(Context))0x400000UL; // entry1
  c1.i = 0;
  
  c1 = c1.next(c1); // call entry1

  show_i(f2h, c1.i);
  
  if (c1.i != 1) {
    return_value(f2h, 1);
    return 1;
  }

  Context c2;
  c2.next = (Context (*)(Context))0x500000UL; // entry3
  c2.i = 0;
  
  c2 = c2.next(c2); // call entry3

  show_i(f2h, c2.i);
  
  if (c2.i != 9) {
    return_value(f2h, 1);
    return 1;
  }

  c1 = c1.next(c1); // call entry2

  show_i(f2h, c1.i);

  if (c1.i != 2) {
    return_value(f2h, 1);
    return 1;
  }

  c2 = c2.next(c2); // call entry4

  show_i(f2h, c2.i);

  if (c2.i != 8) {
    return_value(f2h, 1);
    return 1;
  }

  puts(f2h, "bye!");

  return_value(f2h, 0);
  
  return 0;
}
