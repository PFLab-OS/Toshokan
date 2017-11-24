#include <linux/cpu.h>

#include "cpu_hotplug.h"

static int unpluged_cpu;
static void __init select_unplug_cpu(void);

int __init cpu_unplug(void)
{
    int ret;

    select_unplug_cpu();

    ret = cpu_down(unpluged_cpu);
    if (ret < 0) {
        return ret;
    }

    return unpluged_cpu;
}

int __exit cpu_replug(void)
{
    int ret = cpu_up(unpluged_cpu);
    if (ret < 0) {
        return ret;
    }

    return unpluged_cpu;
}

static void __init select_unplug_cpu(void)
{
    // TODO: 環境・状況によって適切なCPUを選ぶ。
    //       現在はCPUが2つあることを仮定し、決め打ちでCPU1をunplugしている。

    unpluged_cpu = 1;
}
