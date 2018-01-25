#include "time.h"

void wait_usec(unsigned int us)
{
    uint32_t freq = syscounter_freq();
    uint64_t end = syscounter_val() + ((freq / 1000) * us) / 1000;

    while (syscounter_val() < end)
        ;
}
