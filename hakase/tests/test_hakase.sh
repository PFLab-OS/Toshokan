#!/bin/bash -e
# arg1: binary file name
# arg2...: arguments

cd `dirname $0`
if [ -f $1 ]; then
    zc=""
    while [ "$zc" != "0" ]
    do
      zc=`sudo cat /sys/kernel/debug/friend_loader/zero_clear`
    done

    rval=0
    sudo ./test_library.sh $* || rval=$?

    exit $rval
else
    # will be fail in test_library.sh
    ./test_library.sh $*
fi
