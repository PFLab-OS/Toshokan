#!/bin/bash -e

if [ $# -ge 1 ]; then
    pushd ../FriendLoader
    ./run.sh run
    popd

    sudo $1
    rval=$?
    
    pushd ../FriendLoader
    ./run.sh stop
    popd
    exit $rval
else
    exit 1
fi
