#pragma once

#include <toshokan/symbol.h>
#include <toshokan/export.h>

extern int SHARED_SYMBOL(sync_flag);
extern int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
