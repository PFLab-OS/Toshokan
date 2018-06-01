#!/bin/bash -e
# arg1: If crash test or not.
# arg2: binary file name
# arg3...: arguments

cd `dirname $0`
if [ -f $2 ]; then
    pushd ../FriendLoader
    ./run.sh run
    popd

    trap 'pushd ../FriendLoader; ./run.sh stop; popd' SIGINT

    sudo ./test_library.sh $*
    rval=$?

    trap SIGINT
    
    pushd ../FriendLoader
    ./run.sh stop
    popd
    exit $rval
else
    # will be fail in test_library.sh
    ./test_library.sh $*
fi
