#include <asm/io.h>
#include <linux/kernel.h>

#include "common.h"
#include "deploy_data.h"

static void write_phys_mem(phys_addr_t addr, const void* data, size_t size);

int deploy_data(const char* data, size_t size, loff_t offset)
{
    if (DEPLOY_PHYS_ADDR_START + offset + size > DEPLOY_PHYS_ADDR_END) {
        pr_warn("deploy_data: file size too large\n");
        return -1;
    }

    write_phys_mem(DEPLOY_PHYS_ADDR_START + offset, data, size);
    // pr_debug("deploy_data: deployed to physical memory [%llx - %llx] (%zu)\n",
    //     DEPLOY_PHYS_ADDR_START + offset,
    //     DEPLOY_PHYS_ADDR_START + offset + size,
    //     size);

    return 0;
}

static void write_phys_mem(phys_addr_t addr, const void* data, size_t size)
{
    void __iomem* io_addr = ioremap(addr, size);
    memcpy_toio(io_addr, data, size);
    iounmap(io_addr);
}
