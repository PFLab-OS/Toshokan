#pragma once
#include <stdint.h>
#include "common/channel_accessor.h"

namespace Debug {
  extern uint8_t channel_read_buffer[kBufMax];
  extern uint8_t channel_write_buffer[kBufMax];
  void InitChannelBuffer();
}
