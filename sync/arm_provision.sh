#!/bin/bash

test -f /etc/arm_qemu_guest && exit

sudo sed -i'~' -E \
    "s@http://(..\.)?(archive|security)\.ubuntu\.com/ubuntu@http://linux.yz.yamagata-u.ac.jp/pub/linux/ubuntu-archive/@g" \
    /etc/apt/sources.list
sudo DEBIAN_FRONTEND=noninteractive apt -qq update

sudo DEBIAN_FRONTEND=noninteractive apt -qq install -y \
    build_essential

sudo sed -i "s/127.0.0.1 localhost/127.0.0.1 localhost ubuntu/" /etc/hosts

sudo sh -c 'date > /etc/arm_qemu_guest'
