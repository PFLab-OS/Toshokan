#include <toshokan/friend/offload.h>
#include "shared.h"

int SHARED_SYMBOL(value) = 0;

void friend_main() {
  int x = 0;
  OFFLOAD({
    x = EXPORTED_SYMBOL(func)();
    EXPORTED_SYMBOL(printf)
    ("%s\n", "Hello World!");
  });
  SHARED_SYMBOL(value) = x;
}
