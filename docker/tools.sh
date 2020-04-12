#!/bin/sh -ex
apk add --no-cache rsync
mkdir -p /usr/local/share/misc/
cp -r /share/tools /usr/local/share/misc/toshokan
