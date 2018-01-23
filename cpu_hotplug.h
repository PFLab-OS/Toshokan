#pragma once

#include <linux/init.h>   // __init
#include <linux/types.h>  // phys_addr_t

// Unplug CPU.
// Returns unpluged CPU number, or negative number on error.
int __init cpu_unplug(void);

int cpu_start(phys_addr_t entry_point);

// Replug CPU.
// Returns repluged CPU number, or negative number on error.
int __exit cpu_replug(void);
