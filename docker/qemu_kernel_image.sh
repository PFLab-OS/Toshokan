#!/bin/sh -ex
sed -i.bak  -s "s%http://archive.ubuntu.com/ubuntu/%http://ftp.jaist.ac.jp/pub/Linux/ubuntu/%g"  /etc/apt/sources.list
sed -i.bak  -s "s%http://security.ubuntu.com/ubuntu/%http://ftp.jaist.ac.jp/pub/Linux/ubuntu/%g"  /etc/apt/sources.list
apt update
apt install -y initramfs-tools linux-image-4.13.0-45-generic cpio
mkdir /tmp/initramfs
cd /tmp/initramfs
zcat /boot/initrd.img-4.13.0-45-generic | cpio -id
cp -r lib/modules /share/
cp -r /boot/vmlinuz-4.13.0-45-generic /share/
