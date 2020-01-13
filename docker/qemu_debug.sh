#!/bin/bash -ex
apt update -y
apt install -y openssh-client rsync busybox-static
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

cp -r /share/vmlinuz-4.13.0-45-generic /
cp -r /share/rootfs /
mkdir build
cp /share/friend_loader.ko build
qemu-img create -f qcow2 /backing.qcow2 5G
qemu-system-x86_64 -cpu Haswell -d cpu_reset -no-reboot -smp 5 -m 4G -D /tmp/qemu.log -hda /backing.qcow2 -kernel /vmlinuz-4.13.0-45-generic -initrd /rootfs -append 'root=/dev/ram rdinit=/sbin/init memmap=0x70000$4K memmap=0x40000000$0x40000000 console=ttyS0,115200' -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic -global hpet.msi=true > /dev/null 2>&1 &
while ! sh -c "rsync --list-only toshokan_qemu: > /dev/null 2>&1" ; do
    sleep 1;
done
rsync build/* toshokan_qemu:.
ssh toshokan_qemu sudo insmod friend_loader.ko
echo 'stop' | busybox nc localhost 4445
echo 'savevm snapshot1' | busybox nc localhost 4445
echo 'quit' | busybox nc localhost 4445

cp /mnt/wait-for-rsync /usr/local/bin/wait-for-rsync
chmod +x /usr/local/bin/wait-for-rsync

