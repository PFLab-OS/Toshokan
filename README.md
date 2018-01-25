# Friend Loader

## Requirements

* Run on HiKey960
* Add `mem=2G` to the Linux boot parameter

## Usage

### 1. Convert ELF File to Friend Process Image

```console
$ cd ELFtoImg
$ make
$ ./main [ELF file] <output img file>
```

### 2. Build & Install Friend Loader

#### TL; DR

```console
$ KERN_SRC=<linux kernel source directory> ./run.sh <img file>
```

#### Step by step

<details>
<summary> Expand </summary>

Build & insmod.

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
$ cat file | sudo dd of=/dev/depftom
```
</details>

### 3. Boot the Friend Core

```console
$ echo 1 | sudo tee /sys/module/depftom/boot/boot
```

## TODO

*
*
*
