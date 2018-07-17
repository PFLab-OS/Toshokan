#!/bin/bash
# arg1: binary file name
# arg2...: arguments

cd `dirname $0`
if [ -f $1 ]; then
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
