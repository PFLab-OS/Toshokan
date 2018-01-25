#include "uart.h"

volatile unsigned int* const UART0_PTR = (unsigned int*)0xfff32000;

void print_char(char c) { *UART0_PTR = (unsigned int)c; }

void print_str(const char* str)
{
    while (*str != '\0') {
        print_char(*str);
        str++;
    }
}

void print_int(int x)
{
    int i = 0;
    char x_str[32] = {'\0'};

    int negative = x < 0;

    if (negative)
        x = -x;

    for (; x > 0; x /= 10, i++)
        x_str[i] = (char)('0' + x % 10);

    if (negative)
        print_char('-');

    if (i == 0) {
        print_char('0');
    } else {
        for (i -= 1; i >= 0; i--)
            print_char(x_str[i]);
    }
}
