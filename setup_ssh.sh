#!/bin/bash

set -e

# +-------+    +- VirtualBox ---------+
# |       |    |           +- QEMU -+ |
# | Local | -> | Ubuntu -> |  ARM   | |
# |       |    |           | Ubuntu | |
# +-------+    |           +--------+ |
#              +----------------------+

# Local -> Ubuntu on VirtualBox
vagrant ssh-config > .ssh_config

# Ubuntu on VirtualBox -> ARM Ubuntu on QEMU
tmp=$(mktemp)
cat > $tmp << EOF
Host arm
    HostName 127.0.0.1
    User vagrant
    Port 2222
    UserKnownHostsFile /dev/null
    StrictHostKeyChecking no
    PasswordAuthentication no
    IdentitiesOnly yes
    LogLevel FATAL

EOF
scp -F .ssh_config $tmp default:~/.ssh/config

# Local -> ARM Ubuntu on QEMU
scp -F .ssh_config ~/.ssh/id_rsa.pub default:~/id_rsa.pub.local
## scp does not work?
ssh -F .ssh_config default "cat ~/id_rsa.pub.local | ssh arm 'cat >> ~/.ssh/authorized_keys'; rm id_rsa.pub.local"

cat >> .ssh_config << EOT
Host arm
    HostName 127.0.0.1
    User vagrant
    Port 2222
    UserKnownHostsFile /dev/null
    StrictHostKeyChecking no
    PasswordAuthentication no
    IdentitiesOnly yes
    LogLevel FATAL
    ProxyCommand ssh -F .ssh_config default -W %h:%p
EOT

# Test proxy
cmp <(ssh -F .ssh_config arm uname -m) <(echo aarch64)
