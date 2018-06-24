#include <asm/uv/uv.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>

#include "common.h"
#include "cpu_hotplug.h"
#include "deploy.h"
#include "trampoline_loader.h"

static struct trampoline_region tregion;
static int *unplugged_cpu_list = NULL;

int cpu_unplug(void) {
  int i, ret1, ret2, cpu;
  ret1 = 0;

  if (unplugged_cpu_list == NULL) {
    unplugged_cpu_list = (int *)kmalloc(sizeof(int) * num_possible_cpus(), GFP_KERNEL);
    if (!unplugged_cpu_list) {
      return -1;
    }
    memset(unplugged_cpu_list, -1, sizeof(int) * num_possible_cpus());

    get_online_cpus();

    i = 0;
    for_each_online_cpu(cpu) {
      if (cpu != 0 && cpu_is_hotpluggable(cpu)) {
        unplugged_cpu_list[i] = cpu;
        i++;
      }
    }
    
    put_online_cpus();
  }

  for(i = 0; i < num_possible_cpus(); i++) {
    if (unplugged_cpu_list[i] > 0) {
      ret2 = cpu_down(unplugged_cpu_list[i]);
      if (ret2 < 0) {
        ret1 = -1;
      }
    }
  }

  return ret1;
}

int cpu_start() {
  int apicid;
  int boot_error;

  if (trampoline_region_alloc(&tregion) < 0) {
    return -1;
  }

  pr_info("friend_loader: allocate trampoline region at 0x%llx\n",
          tregion.paddr);

  if (trampoline_region_init(&tregion, DEPLOY_PHYS_ADDR_START,
                             DEPLOY_PHYS_ADDR_END) < 0) {
    return -1;
  }

  apicid = apic->cpu_present_to_apicid(1);

  if (get_uv_system_type() != UV_NON_UNIQUE_APIC) {
    smpboot_setup_warm_reset_vector(tregion.paddr);
  }

  preempt_disable();

  /*
   * Wake up AP by INIT, INIT, STARTUP sequence.
   */
  boot_error = wakeup_secondary_cpu_via_init(apicid, tregion.paddr);

  preempt_enable();

  return boot_error;
}

int cpu_replug(void) {
  int i, ret1, ret2;
  ret1 = 0;
  if (unplugged_cpu_list == NULL) {
    return -1;
  }

  for (i = 0; i < num_possible_cpus(); i++) {
    if (unplugged_cpu_list[i] > 0) {
      ret2 = cpu_up(unplugged_cpu_list[i]);
      if (ret2 < 0) {
        ret1 = -1;
      }
    }
  }

  kfree(unplugged_cpu_list);
  unplugged_cpu_list = NULL;

  return ret1;
}

