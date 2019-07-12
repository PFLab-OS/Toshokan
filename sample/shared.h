#pragma once

#include <stdint.h>
#include <toshokan/export.h>
#include <toshokan/symbol.h>

// initialized by hakase.cc (before friend app is executed)
extern int64_t SHARED_SYMBOL(sync_flag);
extern int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
