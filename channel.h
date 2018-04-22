#pragma once
#include <stdint.h>

#define PAGE_SIZE     4096

int32_t get_type(char *channel) {
  return *((int32_t *)channel);
}

void set_type(char *channel, int32_t type) {
  *((int32_t *)channel) = type;
}

void read(char *channel, int offset, uint8_t &data) {
  data = channel[offset + 4];
}

void write(char *channel, int offset, uint8_t data) {
  channel[offset + 4] = data;
}
