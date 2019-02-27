#!/bin/bash
# arg1: binary file name
# arg2...: arguments

cd `dirname $0`
if [ -f $1 ]; then
    ret=0
    $* || ret=$?
    if [ $ret -eq 0 ]; then
        echo -e "\e[32m$1: PASSED\e[m"
        exit 0
    else
        echo -e "\e[31m$1: FAILED\e[m"
        exit 1
    fi
else
    echo -e "\e[31m$2: FAILED (No such file)\e[m"
    exit 1
fi
   
