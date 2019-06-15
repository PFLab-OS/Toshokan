#include <toshokan/friend/offload.h>
#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);
int SHARED_SYMBOL(state);

void friend_main() {
  OFFLOAD(SHARED_SYMBOL(__toshokan_offloader), { SHARED_SYMBOL(state) = 1; });
  SHARED_SYMBOL(sync_flag) = 1;
}
