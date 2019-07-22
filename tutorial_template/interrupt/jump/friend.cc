#include "shared.h"

volatile uint16_t idtr[5];

void friend_main() {
  //  asm volatile("lidt (%0)" ::"r"(idtr));
  //  asm volatile("sti;");
}
