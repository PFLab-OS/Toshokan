#!/bin/sh -x

docker build -t toshokan_qemu_back - <<EOF
FROM livadk/toshokan_qemu:$1 AS build-snapshot
RUN qemu-img create -f qcow2 /backing.qcow2 5G
COPY --from=livadk/toshokan_ssh:$1 /id_rsa /
RUN apk add openssh-client
RUN sh -c "qemu-system-x86_64 -cpu Haswell -d cpu_reset -no-reboot -smp 5 -m 4G -D /tmp/qemu.log -hda /backing.qcow2 -kernel /vmlinuz-4.13.0-45-generic -initrd /rootfs -append 'root=/dev/ram rdinit=/sbin/init memmap=0x40000000\\\$0x40000000 console=ttyS0,115200' -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic > /dev/null 2>&1 &" \
 && sh -c "while ! sh -c 'echo exit | sftp -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -P 2222 hakase@localhost > /dev/null 2>&1' ; do sleep 1; done" \
 && sh -c "echo 'stop' | nc localhost 4445" \
 && sh -c "echo 'savevm snapshot1' | nc localhost 4445" \
 && sh -c "echo 'quit' | nc localhost 4445"

FROM livadk/toshokan_qemu:$1
COPY --from=build-snapshot /backing.qcow2 /
CMD qemu-system-x86_64 -cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -kernel /vmlinuz-4.13.0-45-generic -initrd /rootfs -append "root=/dev/ram rdinit=/sbin/init memmap=0x40000000\\\$0x40000000 console=ttyS0,115200" -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic
EXPOSE 2222 4444 4445 1234
EOF
