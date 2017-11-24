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

or

```shell
$ make qemu
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

## Provision ARM Ubuntu

After first login to ARM Ubuntu, execute `~/sync/arm_provision.sh` to provision.
Then please logout and login back.

## Sync files between local and ARM Ubuntu

A command

```shell
$ make arm-sync
```

... synchonizes `sync` directory between local environment and ARM Ubuntu on QEMU.

This way is tentative. The sharing will be automated in the future.

[SSH is configured]: #configure-ssh
[starting QEMU]: #start-qemu
