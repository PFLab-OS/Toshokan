#!/bin/sh -ex
apk add --no-cache rsync
cp -r /share/hakase_include/* /usr/local/include/
cp /share/lib/libhakase.a /usr/local/lib64
cp /share/lib/libcommon.a /usr/local/lib64
cp /share/hakase/hakase.ld /usr/local/etc
mkdir -p /usr/local/share/misc/
cp -r /share/tools /usr/local/share/misc/toshokan
