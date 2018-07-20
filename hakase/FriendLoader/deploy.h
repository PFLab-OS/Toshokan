#pragma once

#include <linux/types.h>  // loff_t

// Deploy `data` with size `size` to physical memory.
int deploy(const char *data, size_t size, loff_t offset);
// read data from physical memory
int read_deploy_area(char *data, size_t size, loff_t offset);
// zero clear physical memory
int deploy_zero(loff_t offset, size_t size);
