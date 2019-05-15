#!/bin/sh -ex
mkdir -p /usr/local/include/toshokan
mkdir -p /usr/local/etc
cp -r /share/include/* /usr/local/include/toshokan
cp /share/hakase/hakase.ld /usr/local/etc
