#pragma once

#include <linux/types.h> // loff_t

// Deploy `data` with size `size` to physical memory.
int deploy(const char *data, size_t size, loff_t offset);
// zero clear physical memory
int deploy_zero(size_t size, loff_t offset);
