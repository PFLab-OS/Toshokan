#pragma once

// Time measuring
#include <linux/time.h>

typedef struct timespec ts_t;

static inline long ts_diff_ns(const ts_t *start, const ts_t *end) {
  static const long E9 = 1000 * 1000 * 1000;
  long diff_ns = 0;

  if (end->tv_nsec < start->tv_nsec) {
    BUG_ON(end->tv_sec <= start->tv_sec);
    diff_ns += (end->tv_sec - start->tv_sec - 1) * E9;
    diff_ns += end->tv_nsec + E9 - start->tv_nsec;
  } else {
    diff_ns += (end->tv_sec - start->tv_sec) * E9;
    diff_ns += end->tv_nsec - start->tv_nsec;
  }

  return diff_ns;
}
