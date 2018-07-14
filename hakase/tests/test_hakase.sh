#!/bin/bash -e
# arg1: If crash test or not.
# arg2: binary file name
# arg3...: arguments

cd `dirname $0`
if [ -f $2 ]; then
    ./run.sh run

    trap './run.sh stop;' SIGINT

    sudo ./test_library.sh $*
    rval=$?

    trap SIGINT
    
    ./run.sh stop
    exit $rval
else
    # will be fail in test_library.sh
    ./test_library.sh $*
fi
