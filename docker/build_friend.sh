#!/bin/sh -ex
mkdir -p /usr/local/include/toshokan
mkdir -p /usr/local/include/friend
mkdir -p /usr/local/etc
cp -r /share/include/* /usr/local/include/toshokan
cp /share/friend/*.h /usr/local/include/friend
cp /share/friend/friend.ld /usr/local/etc

