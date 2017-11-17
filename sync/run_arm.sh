#!/bin/bash

# Note: `pkill qemu` されるので、このスクリプトのファイル名に `qemu` をいれると動かない。

telnet_addr=127.0.0.1
telnet_port=12345

kill_qemu() {
    if pgrep qemu > /dev/null; then
        pkill -KILL qemu
    fi
}

start_qemu() {
    uefi_file=/home/vagrant/QEMU_EFI.fd
    img_file=/home/vagrant/ubuntu-16.04-server-cloudimg-arm64-uefi1.img
    cloud_img_file=/home/vagrant/cloud.img

    qemu-system-aarch64 \
        -smp 2 \
        -m 1024 \
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
        -monitor telnet:$telnet_addr:$telnet_port,server,nowait \
        -virtfs local,path=./sync,mount_tag=arm-sync,security_model=none
}

case "$1" in
"qemu")
    kill_qemu
    start_qemu
    ;;
"telnet")
    telnet $telnet_addr $telnet_port
    kill_qemu
    ;;
*)
    echo "Invalid usege"
    exit 1
esac
