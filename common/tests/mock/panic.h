#pragma once

#define panic(x) panic_func(x)
void panic_func(const char *str = nullptr);
