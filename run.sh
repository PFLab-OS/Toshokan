#!/bin/bash

# Usage: run.sh <file>
#
# `file` is deployed if specified.

set -e

make
sudo insmod depftom.ko

dev_id=$(cat /proc/devices | grep depftom | cut -d ' ' -f 1)
if [ ! -e /dev/depftom ]; then
    sudo mknod /dev/depftom c $dev_id 0
fi

python3 ./create_prog_file.py
cat ./prog.bin | sudo dd of=/dev/depftom

set +x

echo << EOF
# Run the following command to boot the friend core.
$ echo 1 | sudo tee /sys/module/depftom/boot/boot
EOF
