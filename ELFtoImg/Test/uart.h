#pragma once

volatile unsigned int* const UART0_PTR;

void print_char(char);
void print_str(const char*);

void print_int(int);
