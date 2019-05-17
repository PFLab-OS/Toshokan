#pragma once

#include "channel/hakase.h"
#include "common/channel_accessor.h"
#include "type.h"

int16_t get_cpuid();
void puts(F2H &f2h, const char *str);
void return_value(F2H &f2h, int i);
void memset(void *dest, int ch, size_t n);
void memcpy(void *dest, const void *src, size_t n);
