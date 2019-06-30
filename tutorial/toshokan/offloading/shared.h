#pragma once

#include <toshokan/export.h>
#include <toshokan/symbol.h>

extern int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
extern int SHARED_SYMBOL(value);
