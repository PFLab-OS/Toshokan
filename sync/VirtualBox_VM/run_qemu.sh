#!/bin/bash

set -e

TELNET_ADDR=127.0.0.1
TELNET_PORT=12345

kill_qemu() {
    if pgrep qemu; then
        pkill -KILL qemu
    fi
}

start_qemu() {
    UEFI_FILE=/home/vagrant/QEMU_EFI.fd
    IMG_FILE=/home/vagrant/ubuntu-16.04-server-cloudimg-arm64-uefi1.img
    CLOUD_IMG_FILE=/home/vagrant/cloud.img

    qemu-system-aarch64 \
        -smp 2 \
        -m 1024 \
        -M virt \
        -cpu cortex-a57 \
        -bios $UEFI_FILE \
        -nographic \
        -device virtio-blk-device,drive=image \
        -drive if=none,id=image,file=$IMG_FILE \
        -device virtio-blk-device,drive=cloud \
        -drive if=none,id=cloud,file=$CLOUD_IMG_FILE \
        -device virtio-net-device,netdev=user0 \
        -netdev user,id=user0 \
        -redir tcp:2222::22 \
        -monitor telnet:$TELNET_ADDR:$TELNET_PORT,server,nowait \
        -virtfs local,path=./sync,mount_tag=arm-sync,security_model=none
}

telnet_qemu() {
    telnet $TELNET_ADDR $TELNET_PORT
}

kill_qemu
start_qemu &
sleep 0.2
telnet $TELNET_ADDR $TELNET_PORT
kill_qemu
