#include <asm/uv/uv.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <asm/apic.h> // TRAMPOLINE_PHYS_HIGH, TRAMPOLINE_PHYS_LOW
#include <asm/io.h> // phys_to_virt
#include <asm/mc146818rtc.h> // CMOS_WRITE
#include <linux/spinlock.h> // spin_lock_irqsave, spin_unlock_irqrestore
#include <linux/mc146818rtc.h> // spin_lock_irqsave

#include "common.h"
#include "cpu_hotplug.h"
#include "deploy.h"
#include "trampoline_loader.h"
#include "common/_memory.h"

static struct trampoline_region tregion;
static int *unpluged_cpu_list = NULL;

// from linux-4.14.34 arch/x86/kernel/smpboot.c
static inline void smpboot_setup_warm_reset_vector(unsigned long start_eip)
{
  unsigned long flags;

  spin_lock_irqsave(&rtc_lock, flags);
  CMOS_WRITE(0xa, 0xf);
  spin_unlock_irqrestore(&rtc_lock, flags);
  *((volatile unsigned short *)phys_to_virt(TRAMPOLINE_PHYS_HIGH)) =
    start_eip >> 4;
  *((volatile unsigned short *)phys_to_virt(TRAMPOLINE_PHYS_LOW)) =
    start_eip & 0xf;
}

#define UDELAY_10MS_DEFAULT 10000
static unsigned int init_udelay = UDELAY_10MS_DEFAULT;

// from linux-4.14.34 arch/x86/kernel/smpboot.c
static int
wakeup_secondary_cpu_via_init(int phys_apicid, unsigned long start_eip)
{
	unsigned long send_status = 0, accept_status = 0;
	int maxlvt, num_starts, j;

	maxlvt = GET_APIC_MAXLVT(apic_read(APIC_LVR));

	/*
	 * Be paranoid about clearing APIC errors.
	 */
	if (APIC_INTEGRATED(boot_cpu_apic_version)) {
		if (maxlvt > 3)		/* Due to the Pentium erratum 3AP.  */
			apic_write(APIC_ESR, 0);
		apic_read(APIC_ESR);
	}

	pr_debug("Asserting INIT\n");

	/*
	 * Turn INIT on target chip
	 */
	/*
	 * Send IPI
	 */
	apic_icr_write(APIC_INT_LEVELTRIG | APIC_INT_ASSERT | APIC_DM_INIT,
		       phys_apicid);

	pr_debug("Waiting for send to finish...\n");
	send_status = safe_apic_wait_icr_idle();

	udelay(init_udelay);

	pr_debug("Deasserting INIT\n");

	/* Target chip */
	/* Send IPI */
	apic_icr_write(APIC_INT_LEVELTRIG | APIC_DM_INIT, phys_apicid);

	pr_debug("Waiting for send to finish...\n");
	send_status = safe_apic_wait_icr_idle();

	mb();

	/*
	 * Should we send STARTUP IPIs ?
	 *
	 * Determine this based on the APIC version.
	 * If we don't have an integrated APIC, don't send the STARTUP IPIs.
	 */
	if (APIC_INTEGRATED(boot_cpu_apic_version))
		num_starts = 2;
	else
		num_starts = 0;

	/*
	 * Run STARTUP IPI loop.
	 */
	pr_debug("#startup loops: %d\n", num_starts);

	for (j = 1; j <= num_starts; j++) {
		pr_debug("Sending STARTUP #%d\n", j);
		if (maxlvt > 3)		/* Due to the Pentium erratum 3AP.  */
			apic_write(APIC_ESR, 0);
		apic_read(APIC_ESR);
		pr_debug("After apic_write\n");

		/*
		 * STARTUP IPI
		 */

		/* Target chip */
		/* Boot on the stack */
		/* Kick the second */
		apic_icr_write(APIC_DM_STARTUP | (start_eip >> 12),
			       phys_apicid);

		/*
		 * Give the other CPU some time to accept the IPI.
		 */
		if (init_udelay == 0)
			udelay(10);
		else
			udelay(300);

		pr_debug("Startup point 1\n");

		pr_debug("Waiting for send to finish...\n");
		send_status = safe_apic_wait_icr_idle();

		/*
		 * Give the other CPU some time to accept the IPI.
		 */
		if (init_udelay == 0)
			udelay(10);
		else
			udelay(200);

		if (maxlvt > 3)		/* Due to the Pentium erratum 3AP.  */
			apic_write(APIC_ESR, 0);
		accept_status = (apic_read(APIC_ESR) & 0xEF);
		if (send_status || accept_status)
			break;
	}
	pr_debug("After Startup\n");

	if (send_status)
		pr_err("APIC never delivered???\n");
	if (accept_status)
		pr_err("APIC delivery error (%lx)\n", accept_status);

	return (send_status | accept_status);
}


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

  if (trampoline_region_alloc(&tregion) != 0) {
    return -1;
  }

  pr_info("friend_loader: allocate trampoline region at 0x%llx\n",
          tregion.paddr);

  if (trampoline_region_init(&tregion, DEPLOY_PHYS_ADDR_START,
                             DEPLOY_PHYS_ADDR_END) < 0) {
    return -1;
  }

  if (pagetable_init() < 0) {
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
	// wait until kMemoryMapId is written by a friend.
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

  pagetable_clean();

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

  trampoline_region_free(&tregion);

  kfree(unpluged_cpu_list);
  unpluged_cpu_list = NULL;

  return ret1;
}

