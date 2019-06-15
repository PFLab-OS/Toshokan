#pragma once
#include <toshokan/offload.h>
#include <toshokan/symbol.h>

extern Offloader SHARED_SYMBOL(__toshokan_offloader);
#define OFFLOAD_FUNC(func, ...)                  \
  OFFLOAD(SHARED_SYMBOL(__toshokan_offloader), { \
    EXPORTED_SYMBOL(func)(__VA_ARGS__);          \
  })
