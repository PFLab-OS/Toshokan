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

Page SHARED_SYMBOL(__toshokan_pdpt);
Page SHARED_SYMBOL(__toshokan_pd1);
Page SHARED_SYMBOL(__toshokan_pd2);

int SHARED_SYMBOL(__toshokan_sync_hakase_and_friend) = 0;

static int init_count = 0;
static int returned_count = 0;

extern "C" void friend_init() {
  if (__sync_fetch_and_add(&init_count, 1) == 0) {
    for (void (**ctor)() = __init_array_start; ctor < __init_array_end;
         ++ctor) {
      (**ctor)();
    }
  }

  friend_main();

  if (__sync_add_and_fetch(&returned_count, 1) == init_count) {
    for (void (**dtor)() = __fini_array_start; dtor < __fini_array_end;
         ++dtor) {
      (**dtor)();
    }

    SHARED_SYMBOL(__toshokan_sync_hakase_and_friend) = 1;
  }
}
