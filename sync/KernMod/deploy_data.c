#include <asm/io.h>
#include <linux/kernel.h>

#include "common.h"
#include "deploy_data.h"

static void write_phys_mem(phys_addr_t addr, const void* data, size_t size);

int deploy_data(const char* data, size_t size, struct deployment_info* di)
{
    // FIXME: Linuxが物理メモリのはじめ (PHYS_OFFSET) から2GB使い、
    //       その直後に十分な物理メモリがあることを仮定している。

    di->phys_addr = 2LLU * GB;
    di->phys_offset = PHYS_OFFSET;

    write_phys_mem(di->phys_addr + di->phys_offset, data, size);

    return 0;
}

static void write_phys_mem(phys_addr_t addr, const void* data, size_t size)
{
    void __iomem* io_addr = ioremap(addr, size);
    memcpy_toio(io_addr, data, size);
    iounmap(io_addr);
}
