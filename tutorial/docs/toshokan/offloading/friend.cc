#include <toshokan/friend/offload.h>
#include "shared.h"

int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
int SHARED_SYMBOL(sync_flag) = 0;

void friend_main() {
  OFFLOAD({
    EXPORTED_SYMBOL(printf)
    ("%s\n", "Hello World!");
  });
  SHARED_SYMBOL(sync_flag) = 1;
}
