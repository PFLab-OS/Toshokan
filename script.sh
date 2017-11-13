#!/bin/bash

sudo apt install qemu-system-aarch64 cloud-utils
wget http://snapshots.linaro.org/components/kernel/leg-virt-tianocore-edk2-upstream/latest/QEMU-AARCH64/RELEASE_GCC5/QEMU_EFI.fd
wget https://cloud-images.ubuntu.com/releases/16.04/release-20171107.2/ubuntu-16.04-server-cloudimg-arm64-disk1.img
cat > cloud.txt << EOF
#cloud-config
users:
  - name: ghawkins
    ssh-authorized-keys:
      - ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQC...
    sudo: ['ALL=(ALL) NOPASSWD:ALL']
    groups: sudo
    shell: /bin/bash
EOF
cloud-localds --disk-format qcow2 cloud.img cloud.txt
