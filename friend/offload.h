#pragma once
#include <toshokan/friend/export.h>
#include <toshokan/offload.h>
#include <toshokan/symbol.h>

extern Offloader SHARED_SYMBOL(__toshokan_offloader);
#define OFFLOAD(code) \
  _OFFLOAD(SHARED_SYMBOL(__toshokan_offloader), code)
