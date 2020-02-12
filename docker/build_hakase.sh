#!/bin/sh -ex
cp -r /share/hakase_include/* /usr/local/include/
cp /share/lib/libhakase.a /usr/local/lib64
cp /share/lib/libcommon.a /usr/local/lib64
cp /share/hakase/hakase.ld /usr/local/etc
