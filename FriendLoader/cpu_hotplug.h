#pragma once

#include <linux/init.h>
#include <linux/types.h>

// return value:
//   return number of CPUs (>= 0)
//   return error code (< 0) if error.
int get_cpu_num(void);

// Unplug CPU.
// Returns unpluged CPU number, or negative number on error.
int cpu_unplug(void);

int cpu_start(void);

// Replug CPU.
// Returns repluged CPU number, or negative number on error.
int cpu_replug(void);
