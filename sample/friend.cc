#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);
Page SHARED_SYMBOL(pml4t);
Page SHARED_SYMBOL(pdpt);
Page SHARED_SYMBOL(pd);

void friend_main() { __sync_fetch_and_add(&SHARED_SYMBOL(sync_flag), 1); }
