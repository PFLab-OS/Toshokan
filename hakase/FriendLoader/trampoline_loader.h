#pragma once

#include <linux/types.h> // phys_addr_t

struct trampoline_region {
  phys_addr_t paddr;
};

int trampoline_region_alloc(struct trampoline_region *region);
int trampoline_region_init(struct trampoline_region *region,
                           phys_addr_t phys_addr_start,
                           phys_addr_t phys_addr_end);
int trampoline_region_set_id(struct trampoline_region *region, int cpuid, int apicid);
