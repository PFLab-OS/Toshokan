#include <stdint.h>
#include "_memory.h"
#include "channel.h"
#include "int.h"

Idt idt;

int main() {
  idt.SetupGeneric();
  idt.SetupProc();

  asm volatile("int $0");
  asm volatile("int $1");
  asm volatile("int $2");
  asm volatile("int $3");
  asm volatile("int $4");
  asm volatile("int $5");
  asm volatile("int $6");
  asm volatile("int $7");

  asm volatile("int $9");

  asm volatile("int $15");
  asm volatile("int $16");

  asm volatile("int $18");
  asm volatile("int $19");
  asm volatile("int $20");
  asm volatile("int $21");
  asm volatile("int $22");
  asm volatile("int $23");
  asm volatile("int $24");
  asm volatile("int $25");
  asm volatile("int $26");
  asm volatile("int $27");
  asm volatile("int $28");
  asm volatile("int $29");
  asm volatile("int $30");
  asm volatile("int $31");
  asm volatile("int $32");
}
