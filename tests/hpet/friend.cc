#include <toshokan/friend/export.h>
#include <toshokan/friend/offload.h>
#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);
void (*EXPORTED_SYMBOL(func))(int i, int j);
int (*EXPORTED_SYMBOL(printf))(const char *format, ...);

void friend_main() {
  OFFLOAD_FUNC(func, 11, 10);
  uint64_t xxx = *((uint64_t *)0xfed00000);
  OFFLOAD_FUNC(printf, "<%lx>\n", xxx);
  xxx = *((uint64_t *)0xfed00140);
  OFFLOAD_FUNC(printf, "<%lx>\n", xxx);
  SHARED_SYMBOL(sync_flag) = 1;
}
