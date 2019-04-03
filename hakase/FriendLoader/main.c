#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysfs.h>

#include "call_interface.h"
#include "cpu_hotplug.h"
#include "debug_mem.h"
#include "_memory.h"

MODULE_DESCRIPTION("Friend Loader");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Shinichi Awamoto<sap.pcmail@gmail.com>");

static int __init friend_loader_init(void) {
  int ret;

  pr_info("friend_loader_init: init\n");

  if (call_interface_init() < 0) {
    pr_err("friend_loader_init: failed to init call interface\n");
  }
  
  if (debugmem_init() < 0) {
    pr_err("friend_loader_init: failed to init debugmem\n");
    return -1;
  }

  return 0;
  // TODO: refactor this
  // Unplug friend core
  /* ret = cpu_unplug(); */
  /* if (ret < 0) { */
  /*   pr_warn("friend_loader_init: cpu_unplug failed: %d\n", ret); */
  /*   return -1; */
  /* } else { */
  /*   return 0; */
  /* } */
}

static void __exit friend_loader_exit(void) {
  // TODO: refactor this
  /* int ret = cpu_replug(); */
  /* if (ret < 0) { */
  /*   pr_warn("friend_loader_exit: cpu_replug failed: %d\n", ret); */
  /* } */

  debugmem_exit();
  call_interface_exit();

  pr_info("friend_loader_exit: exit\n");
}

static int boot_flag_set(const char *val, struct kernel_param *kp) {
  int n = 0, ret;

  ret = kstrtoint(val, 10, &n);
  if (ret != 0 || n < 0 || n > 1)
    return -EINVAL;

  if (n == 1) {
    if (cpu_start(n) == 0) {
      pr_info("friend_loader: starting processor cores from 0x%lx\n",
              DEPLOY_PHYS_ADDR_START);
    } else {
      pr_warn("friend_loader: cpu_start failed\n");
      return -EIO;
    }
  } else if (n == 0) {
    // TODO: refactor this
    /* pr_info("friend_loader: stop processor cores\n"); */
    /* int ret = cpu_replug(); */
    /* if (ret < 0) { */
    /*   pr_warn("friend_loade_exit: cpu_replug failed: %d\n", ret); */
    /*   return -EIO; */
    /* } */

    /* ret = cpu_unplug(); */
    /* if (ret < 0) { */
    /*   pr_warn("friend_loader: cpu_unplug failed: %d\n", ret); */
    /*   return -EIO; */
    /* } */
  }

  return param_set_int(val, kp);
}

static int boot = 0;
module_init(friend_loader_init);
module_exit(friend_loader_exit);
module_param_call(boot, boot_flag_set, param_get_int, &boot, 0644);
