#include <linux/kernel.h>
#include <linux/sysfs.h>

#include "deploy.h"
#include "deploy_interface.h"

static struct kobject *deploy_sysfs_kobj;

/* static ssize_t */
/* deploy_bin_read(struct file *filp, struct kobject *kobj, */
/*                   struct bin_attribute *ba, */
/*                   char *buf, loff_t pos, size_t size); */

static ssize_t deploy_bin_write(struct file *filp, struct kobject *kobj,
                                struct bin_attribute *bin_attr, char *buf,
                                loff_t pos, size_t size);

static struct bin_attribute deploy_bin_attr = {
    .attr =
        {
            .name = "content", .mode = S_IWUSR,
        },
    .size = 100 * 1024 * 1024, // 100MB
    .write = deploy_bin_write,
};

static struct bin_attribute *deploy_sysfs_bin_attrs[] = {
    &deploy_bin_attr, NULL,
};
static struct attribute_group deploy_sysfs_attr_group = {
    .bin_attrs = deploy_sysfs_bin_attrs,
};

int __init deploy_interface_init(void) {
  int ret;

  /* sysfs for deploy binary */
  deploy_sysfs_kobj =
      kobject_create_and_add("deploy", &THIS_MODULE->mkobj.kobj);
  if (!deploy_sysfs_kobj) {
    pr_warn("friend_loader_init: kobject_create_and_add failed");
    return -1;
  }

  ret = sysfs_create_group(deploy_sysfs_kobj, &deploy_sysfs_attr_group);
  if (ret != 0) {
    kobject_put(deploy_sysfs_kobj);
    pr_warn("friend_loader_init: sysfs_create_group failed: %d\n", ret);
    return -1;
  }

  return 0;
}

void __exit deploy_interface_exit(void) { kobject_put(deploy_sysfs_kobj); }

static ssize_t deploy_bin_write(struct file *filp, struct kobject *kobj,
                                struct bin_attribute *bin_attr, char *buf,
                                loff_t pos, size_t size) {
  if (deploy(buf, size, pos + 0x100000) < 0) {
    pr_warn("friend_loader: deploy failed\n");
    return -EFBIG;
  }

  return size;
}
