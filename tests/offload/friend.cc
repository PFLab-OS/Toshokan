#include <toshokan/friend/offload.h>
#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);
int (*EXPORTED_SYMBOL(func))(int i, int j);

void friend_main() {
  int x;
  OFFLOAD({ x = EXPORTED_SYMBOL(func)(11, 10); });
  SHARED_SYMBOL(sync_flag) = x / 100;
}
