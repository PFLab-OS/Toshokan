#!/bin/bash

# Usage: run.sh <file>
#
# `file` is deployed if specified.

set -e

# make
insmod friend_loader.ko

if [ $# -eq 1 ]; then
    dd if=$1 of=/sys/module/friend_loader/deploy/content
fi

#set +x

echo 1 | tee /sys/module/friend_loader/parameters/boot
