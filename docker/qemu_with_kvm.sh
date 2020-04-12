#!/bin/sh -ex
apk add --no-cache qemu-system-x86_64
cp /share/backing.qcow2 /
cp /mnt/wait-for-rsync /usr/local/bin/wait-for-rsync
chmod +x /usr/local/bin/wait-for-rsync
