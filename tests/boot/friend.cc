#include "preallocated.h"
#include "stddef.h"

extern "C" {
int64_t *sync_flag_addr = &preallocated_mem->sync_flag;
}
