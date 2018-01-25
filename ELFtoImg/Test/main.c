#include "common.h"
#include "time.h"
#include "uart.h"

// volatile uint32_t* const SHARED_MEM_PTR = (uint32_t*)0x1177000;

#define BENCHMARK_LOOP_NUM 256

void c_main(void)
{
    const uint32_t freq = syscounter_freq();
    print_fmt("freq = %u\n", (uint64_t)freq);

    for (int i = 0; i < BENCHMARK_LOOP_NUM; i++) {
        uint64_t cnt = 0;
        const uint64_t end = syscounter_val() + freq;

        while (true) {
            const uint64_t current = syscounter_val();
            if (current > end)
                break;

            cnt++;
        }

        print_fmt("cnt = %u\n", cnt);
    }

    print_str("done\n");
}

void c_entry(void) { c_main(); }
