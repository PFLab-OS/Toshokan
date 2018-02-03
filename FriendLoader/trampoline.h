#pragma once

struct trampoline_region {
  dma_addr_t paddr;
  uint32_t* vaddr;
};

int trampoline_region_alloc(struct trampoline_region *region);
void trampoline_region_free(struct trampoline_region *region);



