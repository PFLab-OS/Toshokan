#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Deploy File to Memory");
MODULE_LICENSE("GPL");

static char* file_name = "";
module_param(file_name, charp, S_IRUGO);
MODULE_PARM_DESC(file_name, "File name to be deployed");

static int __init depftom_init(void)
{
    int ret = cpu_down(1 /* TODO */);
    if (ret < 0) {
        pr_warn("depftom_init: cpu_down failed: %d\n", ret);
        return -1;
    }

    pr_info("depftom_init: cpu %d down", 1);
    pr_info("depftom_init: \"%s\"", file_name);

    return 0;
}

static void __exit depftom_exit(void)
{
    int ret = cpu_up(1);
    if (ret < 0) {
        pr_warn("depftom_exit: cpu_up failed: %d\n", ret);
    }

    pr_info("depftom_exit: cpu %d up", 1);
}

module_init(depftom_init);
module_exit(depftom_exit);
