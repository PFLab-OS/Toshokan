#include <linux/kernel.h>
#include <linux/module.h>

#include "dumper_dev.h"

MODULE_DESCRIPTION("Friend Dumper");
MODULE_LICENSE("GPL v2");

static int __init friend_dumper_init(void)
{
    pr_info("friend_dumper_init: init\n");
    dumper_dev_init();

    return 0;
}

static void __exit friend_dumper_exit(void)
{
    dumper_dev_exit();
    pr_info("friend_dumper_exit: exit\n");
}

module_init(friend_dumper_init);
module_exit(friend_dumper_exit);
