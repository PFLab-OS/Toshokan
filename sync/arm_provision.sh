#!/bin/bash

test -f /etc/arm_qemu_guest && exit

sudo sed -i'~' -E \
    "s@http://(..\.)?(archive|security)\.ubuntu\.com/ubuntu@http://linux.yz.yamagata-u.ac.jp/pub/linux/ubuntu-archive/@g" \
    /etc/apt/sources.list
sudo DEBIAN_FRONTEND=noninteractive apt -qq update

sudo DEBIAN_FRONTEND=noninteractive apt -qq install -y \
    make \
    gcc \
    gdb

sudo sed -i "s/127.0.0.1 localhost/127.0.0.1 localhost ubuntu/" /etc/hosts

# Restrict amount of memory Linux can recognize and use.
# Total amount should be specified in `qemu.sh`.
sudo sed -i -e \
    '/^GRUB_CMDLINE_LINUX_DEFAULT=/s/"$/ mem=2048M memmap=2048M$2048M"/' \
    /etc/default/grub
sudo update-grub

sudo sh -c 'date > /etc/arm_qemu_guest'
