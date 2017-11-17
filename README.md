## System

```
+-------+    +- VirtualBox ---------+
|       |    |           +- QEMU -+ |
| Local | -> | Ubuntu -> |  ARM   | |
|       |    |           | Ubuntu | |
+-------+    |           +--------+ |
             +----------------------+
```

## Build VirtualBox VM

```shell
$ vagrant up
```

## Start QEMU

```shell
$ vagrant ssh
$ ~/sync/qemu.sh
```

or after [SSH is configured],

```shell
$ ssh -F .ssh_config default '~/sync/qemu.sh'
```

Wait until `Cloud-init v. X.Y.Z finished at ...` appers.
You cannot login with username and password.

## Login to ARM Ubuntu on QEMU

On another local terminal,

```shell
$ vagrant ssh
$ ssh -p 2222 vagrant@localhost
```

or after [SSH is configured],

```shell
$ vagrant ssh
$ ssh arm
```

or

```shell
$ ssh -F .ssh_config arm
```

## Configure SSH

After [starting QEMU], on local terminal,

```shell
$ ./setup_ssh.sh
```

[SSH is configured]: #configure-ssh
[starting QEMU]: #start-qemu
