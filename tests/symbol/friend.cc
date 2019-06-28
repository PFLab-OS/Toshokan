#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);
int *SHARED_SYMBOL(notify);
int *EXPORTED_SYMBOL(hakase_var);

void friend_main() {
  SHARED_SYMBOL(notify) = EXPORTED_SYMBOL(hakase_var);
  __sync_fetch_and_add(&SHARED_SYMBOL(sync_flag), 1);
}
