# Friend Loader

## Requirements

* Add `mem=2G` to the Linux boot parameter
<!--
## Usage

### 1. Convert ELF File to Friend Process Image

```console
$ cd ELFtoImg
$ make
$ ./main [ELF file] <output img file>
```

Default `output img file` is `img.bin`.

### 2. Build & Install Friend Loader

#### TL; DR

```console
$ cd FriendLoader
$ KERN_SRC=<linux kernel source directory> ./run.sh <img file>
```

`img file` is deployed if specified.

#### Step by step

<details>
<summary> Expand </summary>

Build & insmod.

```console
$ cd FriendLoader
$ KERN_SRC=<linux kernel source directory> make
$ sudo insmod friend_loader.ko
```

Create `/dev/friend_loader`.

```console
$ sudo mknod /dev/friend_loader c $(cat /proc/devices | grep friend_loader | cut -d ' ' -f 1) 0
```

Deploy file to physical memory.

```console
$ cat [file] | sudo dd of=/dev/friend_loader
```
</details>

### 3. Boot the Friend Core

```console
$ echo 1 | sudo tee /sys/module/friend_loader/boot/boot
```

## TODO

*
*
*
-->