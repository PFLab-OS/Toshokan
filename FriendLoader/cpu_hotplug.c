#include <linux/cpu.h>
#include <asm/uv/uv.h>

#include "cpu_hotplug.h"
#include "trampoline.h"

static int unpluged_cpu = -1;
static void select_unplug_cpu(void);

static struct trampoline_region tregion;

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

int cpu_start(phys_addr_t entry_point)
{
  int apicid;
  int boot_error;
  dma_addr_t paddr;

  if (unpluged_cpu == -1) {
    return -1;
  }

  if (trampoline_region_alloc(&tregion) < 0) {
    return -1;
  }

  apicid = apic->cpu_present_to_apicid(unpluged_cpu);

  if (get_uv_system_type() != UV_NON_UNIQUE_APIC) {
    smpboot_setup_warm_reset_vector(tregion.paddr);
  }

  preempt_disable();

  /*
   * Wake up AP by INIT, INIT, STARTUP sequence.
   */
  boot_error = wakeup_secondary_cpu_via_init(apicid, paddr);

  preempt_enable();

  return boot_error;
}

int __exit cpu_replug(void)
{
    int ret = cpu_up(unpluged_cpu);
    if (ret < 0) {
        return ret;
    }

    return unpluged_cpu;
}

static void select_unplug_cpu(void)
{
    // TODO: 環境・状況によって適切なCPUを選ぶ。
    //       現在はCPUが8つあることを仮定し、決め打ちでCPU7をunplugしている。

    unpluged_cpu = 7;
}
