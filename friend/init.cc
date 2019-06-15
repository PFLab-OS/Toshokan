#include <toshokan/friend/offload.h>

void friend_main();

extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);
extern void (*__fini_array_start[])(void);
extern void (*__fini_array_end[])(void);

Offloader SHARED_SYMBOL(__toshokan_offloader);

struct Page {
  uint64_t entry[512];
} __attribute__((aligned(4096)));

Page SHARED_SYMBOL(pml4t);
Page SHARED_SYMBOL(pdpt);
Page SHARED_SYMBOL(pd);

extern "C" void friend_init() {
  for (void (**ctor)() = __init_array_start; ctor < __init_array_end; ++ctor) {
    (**ctor)();
  }

  friend_main();

  for (void (**dtor)() = __fini_array_start; dtor < __fini_array_end; ++dtor) {
    (**dtor)();
  }
}
