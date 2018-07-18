#include "debug.h"
#include "assert.h"
#include <stdlib.h>
#include "CppUTestExt/MockSupport.h"

uint8_t Debug::channel_read_buffer[kBufMax];
uint8_t Debug::channel_write_buffer[kBufMax];
void Debug::InitChannelBuffer() {
  for(int i = 0; i < kBufMax; i++) {
    channel_read_buffer[i] = rand();
    channel_write_buffer[i] = rand();
  }
}

void assert_func() {
  mock().actualCall("assert");
}
