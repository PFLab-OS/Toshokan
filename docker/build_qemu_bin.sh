#!/bin/sh -ex
sed -i.bak  -s 's%http://archive.ubuntu.com/ubuntu/%http://ftp.jaist.ac.jp/pub/Linux/ubuntu/%g'  /etc/apt/sources.list
sed -i.bak  -s 's%http://security.ubuntu.com/ubuntu/%http://ftp.jaist.ac.jp/pub/Linux/ubuntu/%g'  /etc/apt/sources.list

apt update
apt install -y \
    libglib2.0-dev \
    libfdt-dev \
    libpixman-1-dev \
    libncursesw5-dev \
    zlib1g-dev \
    flex \
    bison \
    wget \
    build-essential
wget --no-check-certificate https://download.qemu.org/qemu-2.12.0.tar.bz2
tar xf qemu-2.12.0.tar.bz2
mkdir build-qemu
cd build-qemu
../qemu-2.12.0/configure --enable-curses --target-list=x86_64-softmmu --static --prefix=/qemu_install
make -j`nproc`
make install
rm -r /qemu_install/var/run
cp -r /qemu_install /mnt/build/
