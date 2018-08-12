#include "app.h"
#include "common.h"

void show_i(F2H &f2h, int i) {
  char mes[] = "i=0";
  mes[2] += i;
  puts(f2h, mes);
}

void create_pagetable(uint64_t *page_structures, uint64_t *pd_for_baremetal, uint64_t page_for_app) {
  // how to use 16KB
  // 0-4KB: PML4T
  // 4KB-8KB: PDPT
  // 8KB-12KB: PD
  // 12KB-16KB: PT
  uint64_t *pml4t = page_structures;
  uint64_t *pdpt = (uint64_t *)((uint64_t)page_structures + 0x1000);
  uint64_t *pd = (uint64_t *)((uint64_t)page_structures + 0x2000);
  uint64_t *pt = (uint64_t *)((uint64_t)page_structures + 0x3000);

  pml4t[0] = ((uint64_t)pdpt + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[0] = ((uint64_t)pd_for_baremetal + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pdpt[3] = ((uint64_t)pd + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pd[0] = ((uint64_t)pt + 0x80000000UL) | (1 << 0) | (1 << 1) | (1 << 2);
  pt[0] = page_for_app | (1 << 0) | (1 << 1) | (1 << 2);
}

int main() {
  F2H f2h;

  puts(f2h, "hello!");

  memset((void *)0x310000UL, 0, 0x9000);

  uint64_t *page_structures_for_app1 = (uint64_t *)0x310000UL; // 16KB (for PML4T, PDPT, PD, PT)
  uint64_t *page_structures_for_app2 = (uint64_t *)0x314000UL; // 16KB (for PML4T, PDPT, PD, PT)
  uint64_t *pd_for_baremetal = (uint64_t *)0x318000UL;

  // page directory of virtual address 0 - 1GB (for friend.cc)
  // shared by app1 and app2
  for (int i = 0; i < 512; i++) {
    pd_for_baremetal[i] = (0x80000000UL + 0x200000UL * i) | (1 << 0) | (1 << 1) | (1 << 2) |
      (1 << 7) | (1 << 8);
  }
  
  create_pagetable(page_structures_for_app1, pd_for_baremetal, 0x80500000UL);
  create_pagetable(page_structures_for_app2, pd_for_baremetal, 0x80600000UL);
  uint64_t *pml4t_for_app1 = page_structures_for_app1;
  uint64_t *pml4t_for_app2 = page_structures_for_app2;

  extern uint8_t _binary_app1_start[];
  extern uint8_t _binary_app1_size[];
  extern uint8_t _binary_app2_start[];
  extern uint8_t _binary_app2_size[];
  memcpy((void *)0x500000UL, (const void *)_binary_app1_start,
         (size_t)_binary_app1_size);
  memcpy((void *)0x600000UL, (const void *)_binary_app2_start,
         (size_t)_binary_app2_size);

  Context c1;
  c1.next = (Context(*)(Context))0xC0000000UL;  // entry1
  c1.i = 0;

  asm volatile("movq %0, %%cr3" ::"r"((uint64_t)pml4t_for_app1 + 0x80000000UL));
  c1 = c1.next(c1);  // call entry1

  show_i(f2h, c1.i);

  if (c1.i != 1) {
    return_value(f2h, 1);
    return 1;
  }

  Context c2;
  c2.next = (Context(*)(Context))0xC0000000UL;  // entry3
  c2.i = 0;

  asm volatile("movq %0, %%cr3" ::"r"((uint64_t)pml4t_for_app2 + 0x80000000UL));
  c2 = c2.next(c2);  // call entry3

  show_i(f2h, c2.i);

  if (c2.i != 9) {
    return_value(f2h, 1);
    return 1;
  }

  asm volatile("movq %0, %%cr3" ::"r"((uint64_t)pml4t_for_app1 + 0x80000000UL));
  c1 = c1.next(c1);  // call entry2

  show_i(f2h, c1.i);

  if (c1.i != 2) {
    return_value(f2h, 1);
    return 1;
  }

  asm volatile("movq %0, %%cr3" ::"r"((uint64_t)pml4t_for_app2 + 0x80000000UL));
  c2 = c2.next(c2);  // call entry4

  show_i(f2h, c2.i);

  if (c2.i != 8) {
    return_value(f2h, 1);
    return 1;
  }

  puts(f2h, "bye!");

  return_value(f2h, 0);

  return 0;
}
