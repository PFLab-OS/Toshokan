#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);

void friend_main() { __sync_fetch_and_add(&SHARED_SYMBOL(sync_flag), 1); }
