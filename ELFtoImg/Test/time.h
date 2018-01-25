#pragma once

#include "common.h"

static inline uint32_t syscounter_freq(void)
{
    uint32_t freq;
    __asm volatile("mrs %0, cntfrq_el0"
                   : "=r"(freq));  // typically 1,920,000
    return freq;
}

static inline uint64_t syscounter_val(void)
{
    uint64_t val;
    __asm__ volatile("mrs %0, cntpct_el0"
                     : "=r"(val));
    return val;
}


void wait_usec(unsigned int us);

static inline void wait_msec(unsigned int ms) { wait_usec(ms * 1000); }
