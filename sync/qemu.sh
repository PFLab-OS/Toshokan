#!/bin/bash

telnet_addr=127.0.0.1
telnet_port=12345

kill_qemu() {
    if pgrep -f qemu-system-aarch64 > /dev/null; then
        pkill -KILL qemu
    fi
}

start_qemu() {
    uefi_file=/home/vagrant/QEMU_EFI.fd
    img_file=/home/vagrant/ubuntu-16.04-server-cloudimg-arm64-uefi1.img
    cloud_img_file=/home/vagrant/cloud.img

    qemu-system-aarch64 \
        -smp 2 \
        -m 4096M \
        -M virt \
        -cpu cortex-a57 \
        -bios $uefi_file \
        -nographic \
        -device virtio-blk-device,drive=image \
        -drive if=none,id=image,file=$img_file \
        -device virtio-blk-device,drive=cloud \
        -drive if=none,id=cloud,file=$cloud_img_file \
        -device virtio-net-device,netdev=user0 \
        -netdev user,id=user0 \
        -redir tcp:2222::22 \
        -monitor telnet:$telnet_addr:$telnet_port,server,nowait
}

kill_qemu
start_qemu &
sleep 0.2
telnet $telnet_addr $telnet_port
kill_qemu
