#!/bin/bash -e
# arg1: binary file name
# arg2...: arguments

cd `dirname $0`
sudo ./test_library.sh $*
