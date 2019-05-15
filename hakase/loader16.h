#pragma once
#include <stdint.h>

/*
DOC START

# friend/loader16
Loader16 manages 16bit startup code.
Loader16 extracts the code to friend16 region(see architecture/trampoline),
and tells the entry address of friend binary to it.

DOC END
*/

class Loader16 {
 public:
  int Init(uint32_t entry);
};
