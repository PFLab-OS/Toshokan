#!/bin/bash -eu

# Usage: run.sh COMMAND
#
# COMMAND := { load | deploy | run | stop | restart | unload }
#
# load: load kernel module
# deploy <file> : deploy a ELF file (Friend OS).
# run: start Friend OS.
# stop: force stop Friend OS.
# unload: unload kernel module

if [ $# -ge 1 ]; then
    case $1 in
	"load" )
	    if [ `lsmod | grep friend_loader | wc -l` -ge 1 ]; then
		sudo rmmod friend_loader.ko
	    fi
	    make all
	    sudo insmod friend_loader.ko
	    ;;
	"deploy" )
	    if [ $# -ge 2 ]; then
		sudo dd if=$2 of=/sys/module/friend_loader/deploy/content
	    else
		echo "error: needs 'file'!"
	    fi
	    ;;
	"run" )
	    sudo sh -c "echo 1 > /sys/module/friend_loader/parameters/boot"
	    ;;
	"stop" )
	    sudo sh -c "echo 0 > /sys/module/friend_loader/parameters/boot"
	    ;;
	"restart" )
	    sudo sh -c "echo 0 > /sys/module/friend_loader/parameters/boot"
	    sudo sh -c "echo 1 > /sys/module/friend_loader/parameters/boot"
	    ;;
	"unload" )
	    sudo sh -c "echo 0 > /sys/module/friend_loader/parameters/boot"
	    sudo rmmod friend_loader.ko
	    ;;
    esac
fi
