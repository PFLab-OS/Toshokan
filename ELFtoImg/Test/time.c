#include "time.h"

uint32_t current_freq(void)
{
    uint32_t freq;
    __asm volatile("mrs %0, cntfrq_el0"
                   : "=r"(freq));  // typically 1,920,000
    return freq;
}

uint64_t current_time(void)
{
    uint64_t val;
    __asm__ volatile("mrs %0, cntpct_el0"
                     : "=r"(val));
    return val;
}

void wait_usec(unsigned int us)
{
    uint32_t freq = current_freq();
    uint64_t end = current_time() + ((freq / 1000) * us) / 1000;

    while (current_time() < end)
        ;
}
