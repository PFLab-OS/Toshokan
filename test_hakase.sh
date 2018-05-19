#!/bin/bash -e

cd `dirname $0`
if [ -f $2 ]; then
    pushd ../FriendLoader
    ./run.sh run
    popd

    sudo ./test_library.sh $*
    rval=$?
    
    pushd ../FriendLoader
    ./run.sh stop
    popd
    exit $rval
else
    ./test_library.sh $*
fi
