#!/bin/sh -ex
apk add wget fakeroot
wget --no-check-certificate https://nl.alpinelinux.org/alpine/v3.8/releases/x86_64/alpine-minirootfs-3.8.0-x86_64.tar.gz
mkdir work
cd /work
fakeroot && tar xf ../alpine-minirootfs-3.8.0-x86_64.tar.gz
chmod 755 .
cp /mnt/inittab etc/inittab
cp /mnt/interfaces etc/network/interfaces
apk add --no-cache --initdb --root . sudo bash openssh-server rsync openrc busybox-initscripts
chroot . addgroup hakase
chroot . adduser -S -s /bin/sh -G hakase hakase
chroot . addgroup hakase wheel
chroot . sh -c "echo '%wheel ALL=(ALL) NOPASSWD: ALL' >> /etc/sudoers"
chroot . sh -c "echo 'hakase:hakase' | chpasswd"
chroot . install -d /etc/runlevels/boot /etc/runlevels/default /etc/runlevels/sysinit /etc/runlevels/shutdown /etc/runlevels/nonetwork
chroot . ln -s /etc/init.d/acpid /etc/runlevels/default
chroot . ln -s /etc/init.d/bootmisc /etc/runlevels/boot
chroot . ln -s /etc/init.d/crond /etc/runlevels/default
chroot . ln -s /etc/init.d/devfs /etc/runlevels/sysinit
chroot . ln -s /etc/init.d/dmesg /etc/runlevels/sysinit
chroot . ln -s /etc/init.d/sshd /etc/runlevels/default
chroot . ln -s /etc/init.d/hostname /etc/runlevels/boot
chroot . ln -s /etc/init.d/hwclock /etc/runlevels/boot
chroot . ln -s /etc/init.d/hwdrivers /etc/runlevels/sysinit
chroot . ln -s /etc/init.d/killprocs /etc/runlevels/shutdown
chroot . ln -s /etc/init.d/loadkmap /etc/runlevels/boot
chroot . ln -s /etc/init.d/mdev /etc/runlevels/sysinit
chroot . ln -s /etc/init.d/modules /etc/runlevels/boot
chroot . ln -s /etc/init.d/mount-ro /etc/runlevels/shutdown
chroot . ln -s /etc/init.d/networking /etc/runlevels/boot
chroot . ln -s /etc/init.d/savecache /etc/runlevels/shutdown
chroot . ln -s /etc/init.d/swap /etc/runlevels/boot
chroot . ln -s /etc/init.d/sysctl /etc/runlevels/boot
chroot . ln -s /etc/init.d/syslog /etc/runlevels/boot
chroot . ln -s /etc/init.d/urandom /etc/runlevels/boot
echo "net.ipv6.conf.all.disable_ipv6 = 1" >> etc/sysctl.d/local.conf
chmod 700 home/hakase
mkdir -m 700 home/hakase/.ssh
cp /mnt/id_rsa.pub home/hakase/.ssh/authorized_keys
chroot . chown hakase:hakase /home/hakase/.ssh
chroot . chown hakase:hakase /home/hakase/.ssh/authorized_keys
chroot . chmod 700 /home/hakase/.ssh
chroot . chmod 600 /home/hakase/.ssh/authorized_keys
cp -r /mnt/build/modules lib/modules
fakeroot && find | cpio --quiet -o -H newc | gzip -9 > ../rootfs
cp /rootfs /mnt/build/
