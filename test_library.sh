#!/bin/bash
# arg1: If crash test or not.
# arg2: binary file name
# arg3...: arguments

if [ -f $2 ]; then
    crash_flag=$1
    shift
    echo $*
    $* > /dev/null 2>&1
    ret=$?
    if [ $crash_flag = 0 ]; then
        if [ $ret -eq 0 ]; then
            mes=1
        else
            mes=0
        fi
    else
        if [ $ret -eq 255 ]; then
            mes=1
        else
            mes=0
        fi
    fi
    if [ $mes = 1 ]; then
        echo -e "\e[32m$1: PASSED\e[m"
    else
        echo -e "\e[31m$1: FAILED\e[m"
    fi
else
    echo -e "\e[31m$1: FAILED\e[m"
fi
   
