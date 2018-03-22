#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysfs.h>

#include "common.h"
#include "cpu_hotplug.h"
#include "deploy_interface.h"

MODULE_DESCRIPTION("Friend Loader");
MODULE_LICENSE("GPL v2");

static int __init friend_loader_init(void)
{
    int ret;

    pr_info("friend_loader_init: init\n");

    // Device for storing program
    deploy_interface_init();

    // Unplug friend core
    ret = cpu_unplug();
    if (ret < 0) {
        pr_warn("friend_loader_init: cpu_unplug failed: %d\n", ret);
        return -1;
    }

    pr_info("friend_loader_init: cpu %d down\n", ret);

    return 0;
}

static void __exit friend_loader_exit(void)
{
    int ret = cpu_replug();
    if (ret < 0) {
        pr_warn("friend_loader_exit: cpu_replug failed: %d\n", ret);
    } else {
        pr_info("friend_loader_exit: cpu %d up\n", ret);
    }

    deploy_interface_exit();

    pr_info("friend_loader_exit: exit\n");
}

static int boot_flag_set(const char *val, struct kernel_param *kp) {
  int n = 0, ret;

  ret = kstrtoint(val, 10, &n);
  if (ret != 0 || n < 0 || n > 2)
    return -EINVAL;

  if (n == 1) {
    if (cpu_start() == 0) {
        pr_info("friend_loader: starting cpu from 0x%lx\n", DEPLOY_PHYS_ADDR_START);
    } else {
        pr_warn("friend_loader: cpu_start failed\n");
    }
  }
  
  return param_set_int(val, kp);
}

static int boot = 0;
module_init(friend_loader_init);
module_exit(friend_loader_exit);
module_param_call(boot, boot_flag_set, param_get_int, &boot, 0644);
