#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag) = 0;

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

Page SHARED_SYMBOL(__toshokan_pml4t);
Page SHARED_SYMBOL(__toshokan_pdpt);
Page SHARED_SYMBOL(__toshokan_pd);

/* dummy */
#include <toshokan/friend/offload.h>

Offloader SHARED_SYMBOL(__toshokan_offloader);
int SHARED_SYMBOL(__toshokan_sync_hakase_and_friend);
