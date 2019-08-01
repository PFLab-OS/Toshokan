#include "shared.h"

int (*SHARED_SYMBOL(notify))();

void friend_main() { SHARED_SYMBOL(notify) = EXPORTED_SYMBOL(func); }
