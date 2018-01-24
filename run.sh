#!/bin/bash

# Usage: run.sh <file>
#
# `file` is deployed if specified.

set -e

sudo insmod depftom.ko

dev_id=$(cat /proc/devices | grep depftom | cut -d ' ' -f 1)
if [ ! -e /dev/depftom ]; then
    sudo mknod /dev/depftom c $dev_id 0
fi

if [ $# -eq 1 ]; then
    cat $1 | sudo dd of=/dev/depftom
fi

set +x

echo << EOF
# Run the following command to boot the friend core.
$ echo 1 | sudo tee /sys/module/depftom/boot/boot
EOF
