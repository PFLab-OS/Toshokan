#!/bin/sh
apt install -y cpio
mkdir /tmp/initramfs
cd /tmp/initramfs
zcat /boot/initrd.img-4.13.0-45-generic | cpio -id
mkdir -p /mnt/build/
cp -r lib/modules /mnt/build/
