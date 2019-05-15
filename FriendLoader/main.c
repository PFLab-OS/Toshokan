#include <linux/kernel.h>
#include <linux/module.h>

#include <toshokan/memory.h>
#include "call_interface.h"
#include "cpudevice.h"

MODULE_DESCRIPTION("Friend Loader");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Shinichi Awamoto<sap.pcmail@gmail.com>");


static int __init friend_loader_init(void) {
  int n;

  pr_info("friend_loader_init: init\n");

  if (call_interface_init() < 0) {
    pr_err("friend_loader_init: failed to init call interface\n");
    return -EIO;
  }

  n = cpudevice_init();
  if (n < 0) {
    return n;
  }
  
  return 0;
}

static void __exit friend_loader_exit(void) {
  call_interface_exit();

  cpudevice_exit();

  pr_info("friend_loader_exit: exit\n");
}

module_init(friend_loader_init);
module_exit(friend_loader_exit);
