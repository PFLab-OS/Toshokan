#include <toshokan/friend/export.h>
#include <toshokan/friend/offload.h>
#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);
void (*EXPORTED_SYMBOL(func))(int i, int j);

void friend_main() {
  OFFLOAD_FUNC(func, 11, 10);
  SHARED_SYMBOL(sync_flag) = 1;
}
