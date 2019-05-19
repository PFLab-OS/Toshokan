#include "shared.h"

int64_t sync_flag;
Page pml4t;
Page pdpt;
Page pd;

extern "C" void friend_main() {
  __sync_fetch_and_add(&sync_flag, 1);
}
