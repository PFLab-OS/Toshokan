#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);

/* dummy */
#include <toshokan/friend/offload.h>

Offloader SHARED_SYMBOL(__toshokan_offloader);

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

Page SHARED_SYMBOL(pml4t);
Page SHARED_SYMBOL(pdpt);
Page SHARED_SYMBOL(pd);
