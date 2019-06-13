#include <toshokan/offload.h>
#include <toshokan/export.h>
#include "shared.h"

int64_t SHARED_SYMBOL(sync_flag);
Page SHARED_SYMBOL(pml4t);
Page SHARED_SYMBOL(pdpt);
Page SHARED_SYMBOL(pd);
Offloader SHARED_SYMBOL(offloader);

void (*EXPORTED_SYMBOL(func))(int i, int j);

inline void *operator new(size_t, void *p) throw() { return p; }

extern "C" void friend_main() {
  new (&SHARED_SYMBOL(offloader)) Offloader;
  OFFLOAD(SHARED_SYMBOL(offloader), { EXPORTED_SYMBOL(func)(11, 10); });
  SHARED_SYMBOL(sync_flag) = 1;
}
