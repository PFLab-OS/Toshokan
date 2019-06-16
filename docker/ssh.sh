#!/bin/sh -ex
apk add --no-cache openssh-client rsync
cp /mnt/wait-for /usr/local/bin/wait-for
chmod +x /usr/local/bin/wait-for
cp /mnt/wait-for-rsync /usr/local/bin/wait-for-rsync
chmod +x /usr/local/bin/wait-for-rsync
mkdir /root/.ssh
cp /mnt/id_rsa /root/.ssh/id_rsa
cp /mnt/config /root/.ssh/config
chmod 700 /root/.ssh
chmod 600 /root/.ssh/config
chmod 600 /root/.ssh/id_rsa
