#!/bin/bash -e
# arg1: If crash test or not.
# arg2: binary file name
# arg3...: arguments

cd `dirname $0`
if [ -f $2 ]; then
    pushd ../FriendLoader > /dev/null 2>&1
    ./run.sh run
    popd > /dev/null 2>&1

    trap 'pushd ../FriendLoader > /dev/null 2>&1; ./run.sh stop; popd > /dev/null 2>&1' SIGINT

    sudo ./test_library.sh $*
    rval=$?

    trap SIGINT
    
    pushd ../FriendLoader > /dev/null 2>&1
    ./run.sh stop
    popd > /dev/null 2>&1
    exit $rval
else
    # will be fail in test_library.sh
    ./test_library.sh $*
fi
