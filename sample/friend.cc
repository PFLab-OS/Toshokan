#include <toshokan/friend/offload.h>
#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag) = 0;

int cnt = 0;

void friend_main() {
  int i = __sync_fetch_and_add(&cnt, 1);
  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("<%d>\n", i);
  });
  __sync_fetch_and_add(&SHARED_SYMBOL(sync_flag), 1);
}
