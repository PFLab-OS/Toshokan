#include <toshokan/friend/export.h>
#include <toshokan/friend/offload.h>
#include "shared.h"

int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
int64_t SHARED_SYMBOL(sync_flag);

int cnt = 0;

void friend_main() {
  /*  int i = __sync_fetch_and_add(&cnt, 1);
  OFFLOAD_FUNC(printf, "<%d>\n", i);
  OFFLOAD_FUNC(printf, "[%d]\n", i);*/
  if (cnt == 0) {
    OFFLOAD_FUNC(printf, " xxx\n");
  }
  __sync_fetch_and_add(&SHARED_SYMBOL(sync_flag), 1);
}
