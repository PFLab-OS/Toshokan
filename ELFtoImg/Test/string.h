#pragma once

typedef long long unsigned int size_t;

static inline void* memcpy(void* dest, const void* src, size_t n)
{
    char* d = (char*)dest;
    const char* s = (const char*)src;

    for (; n > 0; n--, d++, s++) {
        *d = *s;
    }

    return dest;
}

static inline void* memset(void* dest, int _c, size_t n)
{
    char* d = (char*)dest;
    char c = (char)_c;

    for (; n > 0; n--, d++) {
        *d = c;
    }

    return dest;
}
