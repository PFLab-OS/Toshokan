#!/bin/bash -ex
apt update -y
apt install -y openssh-client rsync
apt install -y build-essential zlib1g-dev pkg-config libglib2.0-dev binutils-dev libboost-all-dev autoconf libtool libssl-dev libpixman-1-dev libpython-dev python-pip python-capstone virtualenv git flex bison
mkdir /root/.ssh
cp /mnt/id_rsa /root/.ssh/id_rsa
cp /mnt/config /root/.ssh/config
chmod 700 /root/.ssh
chmod 600 /root/.ssh/config
chmod 600 /root/.ssh/id_rsa

mkdir -p /share/.qemu-build
pushd /share/.qemu-build
/share/qemu/configure --target-list=x86_64-softmmu --enable-debug
make -j8
make install
popd

cp /share/backing.qcow2 /
cp /mnt/wait-for-rsync /usr/local/bin/wait-for-rsync
chmod +x /usr/local/bin/wait-for-rsync

