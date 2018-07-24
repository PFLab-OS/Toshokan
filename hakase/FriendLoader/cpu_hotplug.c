#include <asm/uv/uv.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>

#include "common.h"
#include "cpu_hotplug.h"
#include "deploy.h"
#include "trampoline_loader.h"
#include "common/_memory.h"

static struct trampoline_region tregion;
static int *unpluged_cpu_list = NULL;

int get_cpu_num(void) {
  int i, num = 0;
  if (unpluged_cpu_list == NULL) {
    return -1;
  }
  for(i = 0; i < num_possible_cpus(); i++) {
    if (unpluged_cpu_list[i] > 0) {
      num++;
    }
  }
  return num;
}

int cpu_unplug(void) {
  int i, ret1, ret2, cpu, unpluged_flag;
  ret1 = 0;
  unpluged_flag = 0;

  if (unpluged_cpu_list == NULL) {
    unpluged_cpu_list = (int *)kmalloc(sizeof(int) * num_possible_cpus(), GFP_KERNEL);
    if (!unpluged_cpu_list) {
      return -1;
    }
    memset(unpluged_cpu_list, -1, sizeof(int) * num_possible_cpus());

    get_online_cpus();

    i = 0;
    for_each_online_cpu(cpu) {
      if (cpu != 0 && cpu_is_hotpluggable(cpu)) {
        unpluged_cpu_list[i] = cpu;
        i++;
      }
    }
    
    put_online_cpus();
  }

  for(i = 0; i < num_possible_cpus(); i++) {
    if (unpluged_cpu_list[i] > 0) {
      unpluged_flag = 1;
      ret2 = cpu_down(unpluged_cpu_list[i]);
      if (ret2 < 0) {
        ret1 = -1;
      }
    }
  }

  if (unpluged_flag == 0) {
    pr_warn("friend_loader: no unplugable cpu\n");
    return -1;
  }

  return ret1;
}

int cpu_start() {
  int ret1, ret2;
  int i;
  ret1 = 0;

  if (trampoline_region_alloc(&tregion) < 0) {
    return -1;
  }

  pr_info("friend_loader: allocate trampoline region at 0x%llx\n",
          tregion.paddr);

  if (trampoline_region_init(&tregion, DEPLOY_PHYS_ADDR_START,
                             DEPLOY_PHYS_ADDR_END) < 0) {
    return -1;
  }

  if (get_uv_system_type() != UV_NON_UNIQUE_APIC) {
    smpboot_setup_warm_reset_vector(tregion.paddr);
  }

  preempt_disable();

  /*
   * Wake up AP by INIT, INIT, STARTUP sequence.
   */
  for (i = 0; i < num_possible_cpus(); i++) {
    if (unpluged_cpu_list[i] > 0) {
      int apicid = apic->cpu_present_to_apicid(unpluged_cpu_list[i]);

      if (apicid == 0) {
        ret1 = -1;
        continue;
      }

      if (trampoline_region_set_id(&tregion, i, apicid) < 0) {
	ret1 = -1;
      }

      ret2 = wakeup_secondary_cpu_via_init(apicid, tregion.paddr);
      if (ret2 < 0) {
        ret1 = -1;
      }

      do {
        uint64_t i;
        if (read_deploy_area((char *)&i, sizeof(i), kMemoryMapId) < 0) {
          ret1 = -1;
          break;
        }
        if (i == 0) {
          break;
        }
        asm volatile("pause":::"memory");
      } while(1);
    }
  }

  preempt_enable();

  return ret1;
}

int cpu_replug(void) {
  int i, ret1, ret2;
  ret1 = 0;
  if (unpluged_cpu_list == NULL) {
    return -1;
  }

  for (i = 0; i < num_possible_cpus(); i++) {
    if (unpluged_cpu_list[i] > 0) {
      ret2 = cpu_up(unpluged_cpu_list[i]);
      if (ret2 < 0) {
        ret1 = -1;
      }
    }
  }

  kfree(unpluged_cpu_list);
  unpluged_cpu_list = NULL;

  return ret1;
}

