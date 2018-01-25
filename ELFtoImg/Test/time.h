#pragma once

#include "common.h"

uint32_t current_freq(void);
uint64_t current_time(void);

void wait_usec(unsigned int us);
static inline void wait_msec(unsigned int ms) { wait_usec(ms * 1000); }
