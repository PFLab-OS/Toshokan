#!/bin/bash

# Usage: run.sh <file>
#
# `file` is deployed if specified.

set -e

make
sudo insmod friend_loader.ko

dev_id=$(cat /proc/devices | grep friend_loader | cut -d ' ' -f 1)
if [ ! -e /dev/friend_loader ]; then
    sudo mknod /dev/friend_loader c $dev_id 0
fi

if [ $# -eq 1 ]; then
    cat $1 | sudo dd of=/dev/friend_loader
fi

#set +x

echo 1 | sudo tee /sys/module/friend_loader/boot/boot
