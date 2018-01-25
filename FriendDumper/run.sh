#!/bin/bash

set -e

make
sudo insmod friend_dumper.ko

dev_id=$(cat /proc/devices | grep friend_dumper | cut -d ' ' -f 1)
if [ ! -e /dev/friend_dumper ]; then
    sudo mknod /dev/friend_dumper c $dev_id 0
fi
