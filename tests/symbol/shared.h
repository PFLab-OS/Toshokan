#pragma once

#include <stdint.h>
#include <toshokan/symbol.h>

// initialized by hakase.cc (before friend app is executed)
extern int64_t SHARED_SYMBOL(sync_flag);
extern int *SHARED_SYMBOL(notify);
