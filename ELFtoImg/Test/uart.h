#pragma once

void print_char(char c);
void print_str(const char* str);

/*
 * Supports
 * - %d (int)
 * - %u (uint64_t)
 * - %x (uint64_t)
 * - %b (uint64_t)
 * - %s
 */
void print_fmt(const char* fmt, ...);
