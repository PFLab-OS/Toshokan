#!/bin/sh -ex
sed -i.bak  -s "s%http://archive.ubuntu.com/ubuntu/%http://ftp.jaist.ac.jp/pub/Linux/ubuntu/%g"  /etc/apt/sources.list
sed -i.bak  -s "s%http://security.ubuntu.com/ubuntu/%http://ftp.jaist.ac.jp/pub/Linux/ubuntu/%g"  /etc/apt/sources.list
apt update
apt install -y linux-headers-4.13.0-45-generic gcc make
