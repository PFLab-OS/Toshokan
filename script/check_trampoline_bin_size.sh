#!/bin/sh -e

if [ `wc -c $1 | awk '{print $1}'` -gt `printf "%d" 0x1000` ]; then
    echo "[Fatal Error] The trampoline size is too big!"
    exit 1
fi	
