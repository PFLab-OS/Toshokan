#!/bin/bash -xeu

# Usage: run.sh <file>
#
# `file` is deployed if specified.

make all
sudo insmod friend_loader.ko

if [ $# -eq 1 ]; then
    sudo dd if=$1 of=/sys/module/friend_loader/deploy/content
fi

sudo sh -c "echo 1 > /sys/module/friend_loader/parameters/boot"
