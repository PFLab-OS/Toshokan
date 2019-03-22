#!/bin/sh -ex
apk add --no-cache xterm
cp -r /mnt/build/qemu_install/* /
cp -r /mnt/build/vmlinuz-4.13.0-45-generic /
cp -r /mnt/build/rootfs /
qemu-img create -f qcow2 /backing.qcow2 5G
cp /mnt/id_rsa /
chmod 600 /id_rsa
apk add openssh-client
qemu-system-x86_64 -cpu Haswell -d cpu_reset -no-reboot -smp 5 -m 4G -D /tmp/qemu.log -hda /backing.qcow2 -kernel /vmlinuz-4.13.0-45-generic -initrd /rootfs -append 'root=/dev/ram rdinit=/sbin/init memmap=0x70000$4K memmap=0x40000000$0x40000000 console=ttyS0,115200' -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic > /dev/null 2>&1 &
SSH_OPTION="-o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa"
while ! sh -c "ssh $SSH_OPTION -p 2222 hakase@localhost exit 0 > /dev/null 2>&1" ; do
    sleep 1;
done
echo 'stop' | nc localhost 4445
echo 'savevm snapshot1' | nc localhost 4445
echo 'quit' | nc localhost 4445
