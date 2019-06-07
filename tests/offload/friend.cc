#include "shared.h"
#include <toshokan/offload.h>

int64_t SHARED_SYMBOL(sync_flag);
Page SHARED_SYMBOL(pml4t);
Page SHARED_SYMBOL(pdpt);
Page SHARED_SYMBOL(pd);
Offloader SHARED_SYMBOL(offloader);
int SHARED_SYMBOL(state);

inline void *operator new(size_t, void *p)     throw() { return p; }

extern "C" void friend_main() {
  new(&SHARED_SYMBOL(offloader)) Offloader;
  OFFLOAD(SHARED_SYMBOL(offloader), {SHARED_SYMBOL(state) = 1;});
  SHARED_SYMBOL(sync_flag) = 1;
}
