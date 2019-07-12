#include "shared.h"

int *SHARED_SYMBOL(notify);
int *EXPORTED_SYMBOL(hakase_var);

void friend_main() { SHARED_SYMBOL(notify) = EXPORTED_SYMBOL(hakase_var); }
