#pragma once

#include <linux/types.h>  // phys_addr_t

// Deploy `data` with size `size` to physical memory.
// Related information will be stored to `depinfo`.
int deploy_data(const char* data, size_t size, loff_t offset);
