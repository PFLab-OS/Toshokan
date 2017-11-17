#!/bin/bash

#
# Originating from https://gist.github.com/george-hawkins/16ee37063213f348a17717a7007d2c79
#

# Used for
# 1. avoiding multiple provisioning, and
# 2. determining wheter execution environment is host or VirtualBox guest VM
#    (in Makefile)
test -f /etc/arm_devenv_guest && exit

# Speedup apt
sudo sed -i'~' -E "s@http://(..\.)?(archive|security)\.ubuntu\.com/ubuntu@http://linux.yz.yamagata-u.ac.jp/pub/linux/ubuntu-archive/@g" /etc/apt/sources.list
sudo DEBIAN_FRONTEND=noninteractive apt -qq update

# Install some packages
sudo DEBIAN_FRONTEND=noninteractive apt -qq install -y qemu-system-aarch64 cloud-utils

# Download QEMU_EFI.fd and uefi1.img
wget --progress=bar:force https://releases.linaro.org/components/kernel/uefi-linaro/latest/release/qemu64/QEMU_EFI.fd
wget --progress=bar:force https://cloud-images.ubuntu.com/releases/16.04/release/ubuntu-16.04-server-cloudimg-arm64-uefi1.img

# Generate SSH key
yes "" | ssh-keygen -t rsa -b 4096

# Creat a cloud-config disk image
cat > cloud.txt << EOF
#cloud-config
users:
  - name: vagrant
    ssh-authorized-keys:
      - $(cat ~/.ssh/id_rsa.pub)
    sudo: ['ALL=(ALL) NOPASSWD:ALL']
    groups: sudo
    shell: /bin/bash
EOF
cloud-localds --disk-format qcow2 cloud.img cloud.txt

# Backup image
cp ubuntu-16.04-server-cloudimg-arm64-uefi1.img ubuntu-16.04-server-cloudimg-arm64-uefi1.img.orig

sudo sh -c 'date > /etc/arm_devenv_guest'
