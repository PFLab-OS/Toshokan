#include <asm/io.h>
#include <linux/kernel.h>

#include "deploy_data.h"

static void write_phys_mem(phys_addr_t addr, const void* data, size_t size);

int deploy_data(const char* data, size_t size, loff_t offset)
{
    // XXX: mem=2GでLinuxが起動していると仮定
    phys_addr_t phys_addr_start = 0xba6fc000;
    phys_addr_t phys_addr_end = 0xbf6bffff;

    if (phys_addr_start + offset + size > phys_addr_end) {
        pr_warn("deploy_data: file size too large\n");
        return -1;
    }

    write_phys_mem(phys_addr_start + offset, data, size);
    pr_debug("deploy_data: deployed to physical memory [%llx - %llx]\n",
        phys_addr_start + offset,
        phys_addr_start + offset + size);

    return 0;
}

static void write_phys_mem(phys_addr_t addr, const void* data, size_t size)
{
    void __iomem* io_addr = ioremap(addr, size);
    memcpy_toio(io_addr, data, size);
    iounmap(io_addr);
}
