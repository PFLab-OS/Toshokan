#include <toshokan/export.h>
#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);
Page SHARED_SYMBOL(pml4t);
Page SHARED_SYMBOL(pdpt);
Page SHARED_SYMBOL(pd);

int *SHARED_SYMBOL(notify);

int *EXPORTED_SYMBOL(hakase_var);

extern "C" void friend_main() {
  SHARED_SYMBOL(notify) = EXPORTED_SYMBOL(hakase_var);
  __sync_fetch_and_add(&SHARED_SYMBOL(sync_flag), 1);
}
