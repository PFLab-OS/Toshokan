#pragma once

#include <linux/types.h>  // phys_addr_t

struct deployment_info {
    phys_addr_t phys_addr;  // without offset
    phys_addr_t phys_offset;
};

// Deploy `data` with size `size` to physical memory.
// Related information will be stored to `depinfo`.
int deploy_data(const char* data, size_t size, struct deployment_info* depinfo);
