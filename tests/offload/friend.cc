#include <toshokan/friend/offload.h>
#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag) = 0;

void friend_main() {
  int x;
  OFFLOAD({ x = EXPORTED_SYMBOL(func)(11, 10); });
  SHARED_SYMBOL(sync_flag) = x / 100;
}
