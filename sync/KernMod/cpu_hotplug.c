#include <linux/cpu.h>
#include <linux/kernel.h>
// #include <linux/kprobes.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

// static int cpu_down_entry(unsigned int cpu);

// static struct jprobe cpu_down_jprobe = {
//     .entry = cpu_down_entry,
//     .kp = {
//         .symbol_name = "cpu_down",
//     },
// };

static int cpuhotplug_init(void)
{
    // int ret = register_jprobe(&cpu_down_jprobe);
    // if (ret < 0) {
    //     pr_warn("cpuhotplug_init: register_jprobe failed: %d\n", ret);
    //     return -1;
    // }

    // pr_info("cpuhotplug_init: jprobe registered: %s\n", cpu_down_jprobe.kp.symbol_name);

    int ret = cpu_down(1);
    if (ret < 0) {
        pr_warn("cpuhotplug_init: cpu_down failed: %d\n", ret);
        return -1;
    }

    pr_info("cpuhotplug_init: cpu %d down", 1);

    return 0;
}

static void cpuhotplug_exit(void)
{
    // unregister_jprobe(&cpu_down_jprobe);
    // pr_info("cpuhotplug_exit: jprobe unregistered\n");

    int ret = cpu_up(1);
    if (ret < 0) {
        pr_warn("cpuhotplug_exit: cpu_up failed: %d\n", ret);
    }

    pr_info("cpuhotplug_exit: cpu %d up", 1);
}

module_init(cpuhotplug_init);
module_exit(cpuhotplug_exit);

// static int cpu_down_entry(unsigned int cpu)
// {
//     pr_info("cpu_down_entry: cpu %d down\n", cpu);
//
//     jprobe_return();
//     return 0;
// }
