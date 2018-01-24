# Friend Loader

## Requirements

* Add `mem=2G` to the Linux boot parameter

## Usage

### TL; DR

```console
$ KERN_SRC=<linux kernel source directory> ./run.sh
```

### Step by step

Build & install.

```console
$ KERN_SRC=<linux kernel source directory> make
$ sudo insmod depftom.ko
```

Create `/dev/depftom`.

```console
$ sudo mknod /dev/depftom c $(cat /proc/devices | grep depftom | cut -d ' ' -f 1) 0
```

Deploy file to physical memory.

```console
$ cat depftom.ko | sudo dd of=/dev/depftom
```

Boot the friend core.

```console
$ echo 1 | sudo tee /sys/module/depftom/boot/boot
```

## TODO

*
*
*
