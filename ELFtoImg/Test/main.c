#include "common.h"
#include "time.h"
#include "uart.h"

void c_entry(void)
{
    while (true) {
        print_str("Hello, world!\n");

        uint32_t freq = current_freq();
        print_fmt("Clock freq = %u\n", freq);

        uint64_t time = current_time();
        print_fmt("Clock current = %u\n", time);

        wait_usec(1000 * 1000);
    }
}
