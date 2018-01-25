#include "uart.h"
#include "common.h"

volatile unsigned int* const UART0_PTR = (unsigned int*)0xfff32000;

void print_char(char c)
{
    if (c == '\n')
        print_char('\r');

    int r = 1000;
    while (r--)
        ;

    *UART0_PTR = (unsigned int)c;
}

void print_str(const char* str)
{
    while (*str)
        print_char(*str++);
}

static char digits[] = "0123456789abcdef";

static void print_int(int x, int base, int sign)
{
    char buf[16] = {'\0'};
    int i = 0;

    if (sign && (sign = x < 0))
        x = -x;

    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign)
        buf[i++] = '-';

    while (--i >= 0)
        print_char(buf[i]);
}

static void print_uint64(uint64_t x, uint64_t base)
{
    char buf[24];
    int i = 0;

    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    while (--i >= 0)
        print_char(buf[i]);
}

void print_fmt(const char* fmt, ...)
{
    va_list ap;
    int i;
    char c;
    const char* s;

    va_start(ap, fmt);

    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (c != '%') {
            print_char(c);
            continue;
        }
        c = fmt[++i] & 0xff;
        if (c == 0)
            break;
        switch (c) {
        case 'd':
            print_int(va_arg(ap, int), 10, 1);
            break;
        case 'u':
            print_uint64(va_arg(ap, uint64_t), 10);
            break;
        case 'x':
            print_uint64(va_arg(ap, uint64_t), 16);
            break;
        case 'b':
            print_uint64(va_arg(ap, uint64_t), 2);
            break;
        case 's':
            if ((s = va_arg(ap, char*)) == 0)
                s = "(null)";
            print_str(s);
            break;
        case '%':
            print_char('%');
            break;
        default:
            print_char('%');
            print_char(c);
            break;
        }
    }
}
