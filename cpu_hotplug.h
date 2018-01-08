#pragma once

#include <linux/init.h>  // __init

// Unplug CPU.
// Returns unpluged CPU number, or negative number on error.
int __init cpu_unplug(void);

// Replug CPU.
// Returns repluged CPU number, or negative number on error.
int __exit cpu_replug(void);
