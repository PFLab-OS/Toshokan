# Toshokan
Toshokan provides core-isolated bare-metal application environment (names friend) which solves major difficulties specific to its programming. With running Linux kernel on the first core (names hakase), and assigning other cores for bare-metal applications (friend apps), bare-metal code can call Linux functionalities without any interference from underlying system software. With Toshokan, programmers can now easily measure low-level hardware characteristics, which would inspire new research ideas or give some hints of performance tuning in their software.

## How To Use (in Japanese)
[here](tutorial/toshokan/)

## Install & Test (to build overall Toshokan environment)
Most users do not need to do this. If you just want to write friend app, see [sample](sample/) or [tutorial](tutorial/toshokan/).

requirements:

* docker (tested on 18.06.0-ce)
* scons (> 3.0)
* python (> 2.5)

You can use `pip install scons` to install the latest version of scons.
OS package managers(e.g. apt, yum) often try to install older versions. That would not work.

```
$ git clone https://github.com/PFLab-OS/Toshokan.git
$ cd Toshokan
$ scons -j12
```

## Release Notes
v0.10(2020/04)
- improved the Makefile. Now users do not have to manage the build script.
- The build script strips your app binary file, and produces smaller image.
- -O2 flag is automatically set by default.
- The physical memory region for friend is moved to 4G-5G to support more physical environment.

v0.04(2019/12)
- added tutorial
- fixed some bugs

v0.03(2019/06)
- simplified the way to run Toshokan apps on physical machine



