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

void wait_new_signal(char *channel) {
  while(get_type(channel) == 0) {
    asm volatile("":::"memory");
  }
}

void send_signal(char *channel, int32_t type) {
  set_type(channel, type);

  while(get_type(channel) != 0) {
    asm volatile("":::"memory");
  }
}
