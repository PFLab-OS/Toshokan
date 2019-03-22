#!/bin/sh -ex
apt install -y cpio
mkdir /tmp/initramfs
cd /tmp/initramfs
zcat /boot/initrd.img-4.13.0-45-generic | cpio -id
cp -r lib/modules /mnt/build/
cp -r /boot/vmlinuz-4.13.0-45-generic /mnt/build/
