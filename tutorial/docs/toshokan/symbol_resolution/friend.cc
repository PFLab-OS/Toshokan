#include "shared.h"

int SHARED_SYMBOL(variable) = 0;

void friend_main() {
  SHARED_SYMBOL(variable)++;
}
