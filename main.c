#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "cpu_hotplug.h"
#include "depftom_dev.h"
#include "deploy_data.h"

MODULE_DESCRIPTION("Deploy file to physical memory");
MODULE_LICENSE("GPL v2");

struct trampoline_region {
    phys_addr_t paddr;
    uint32_t* vaddr;
};
static struct trampoline_region region;

static int /* __init */ alloc_trampoline_region(struct trampoline_region* region);
static void __exit free_trampoline_region(struct trampoline_region* region);

static void depftom_up(unsigned long);
// DECLARE_TASKLET(cpu_up_tasklet, depftom_up, 0);

static uint8_t bin[] = {
    0x00, 0x00, 0x84, 0xd2,  // mov x0, 0x2000
    0x60, 0xfe, 0xbf, 0xf2,  // movk x0, 0xfff3, lsl 16
    0x21, 0x08, 0x80, 0x52,  // mov w1, 65
    0x01, 0x00, 0x00, 0x39,  // strb w1, [x0]
    // loop:
    0x5f, 0x20, 0x03, 0xd5,  // wfe
    0xff, 0xff, 0xff, 0x17,  // b loop;
};

static const size_t bin_size = sizeof(bin) / sizeof(bin[0]);

// extern struct ctl_table sysctl_table[];
// static struct ctl_table_header* sysctl_header;

static int /* __init */ depftom_init(void)
{
    pr_info("depftom_init: init\n");

    // Sysctl
    // if ((sysctl_header = register_sysctl_table(sysctl_table)) == NULL) {
    //     pr_warn("depftom_init: failed to register sysctl table\n");
    //     return -1;
    // }

    // Trampoline region
    if (alloc_trampoline_region(&region) < 0) {
        pr_warn("depftom_init: no trampoline space\n");
        return -1;
    }
    memcpy(region.vaddr, bin, bin_size);

    depftom_dev_init();

    depftom_up(0);

    return 0;
}

static void __exit depftom_exit(void)
{
    int ret = cpu_replug();
    if (ret < 0) {
        pr_warn("depftom_exit: cpu_replug failed: %d\n", ret);
    } else {
        pr_info("depftom_exit: cpu %d up\n", ret);
    }

    // tasklet_kill(&cpu_up_tasklet);

    depftom_dev_exit();

    // unregister_sysctl_table(sysctl_header);

    free_trampoline_region(&region);

    pr_info("depftom_exit: exit\n");
}

static int /* __init */ alloc_trampoline_region(struct trampoline_region* region)
{
    if (__friend_loader_buf[0] != FRIEND_LOADER_TRAMPOLINE_SIGNATURE) {
        pr_warn("alloc_trampoline_region: signature does not match\n");
        return -1;
    }

    region->paddr = __pa_symbol(__friend_loader_buf);
    region->vaddr = __friend_loader_buf;

    return 0;
}

static void __exit free_trampoline_region(struct trampoline_region* region) {}

void depftom_up(unsigned long dummy)
{
    int unplugged_cpu = cpu_unplug();

    if (unplugged_cpu < 0) {
        pr_warn("depftom_up: cpu_unplug failed: %d\n", unplugged_cpu);
        return;
    }

    pr_info("depftom_up: cpu %d down\n", unplugged_cpu);

    if (cpu_start(region.paddr) == 0) {
        pr_info("depftom_up: starting cpu from %llx\n", region.paddr);
    } else {
        pr_warn("depftom_up: failed to start cpu\n");
    }

    (void)dummy;
}

// Sysctl
// static uint32_t cpu_up_flag = 0;

// static int proc_doflag(
//     struct ctl_table* table, int write, void __user* buffer,
//     size_t* lenp, loff_t* ppos)
// {
//     int ret = proc_dointvec(table, write, buffer, lenp, ppos);
//
//     if (write && cpu_up_flag == 1) {
//         // TODO
//         // tasklet_schedule(&cpu_up_tasklet);
//     }
//
//     return ret;
// }

// struct ctl_table sysctl_table[] = {
//     {
//         .procname = "depftom",
//         .data = &cpu_up_flag,
//         .maxlen = sizeof(cpu_up_flag),
//         .mode = 0644,
//         .proc_handler = &proc_doflag,
//     },
//     {},
// };

module_init(depftom_init);
module_exit(depftom_exit);
