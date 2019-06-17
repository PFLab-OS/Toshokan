#include <toshokan/friend/export.h>
#include <toshokan/friend/offload.h>
#include "shared.h"

int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
int SHARED_SYMBOL(sync_flag) = 0;

void friend_main() {
  OFFLOAD_FUNC(printf, "%s\n", "Hello World!");
  SHARED_SYMBOL(sync_flag) = 1;
}
