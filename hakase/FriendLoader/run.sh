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
    cat /proc/cmdline | grep "memmap=0x70000\$4K memmap=0x40000000\$0x40000000" > /dev/null 2>&1 || (echo "error: physical memory is not isolated for hakase"; exit 1)
    if [ ! -e friend_loader.ko ]; then
        echo "error: make friend loader kernel module first!"
        exit 1
    fi
    case $1 in
	      "load" )
	          if [ `lsmod | grep friend_loader | wc -l` -ge 1 ]; then
		            sudo rmmod friend_loader.ko
	          fi
	          sudo insmod friend_loader.ko
	          ;;
	      "run" )
	          if [ `lsmod | grep friend_loader | wc -l` -eq 0 ]; then
		            sudo insmod friend_loader.ko
	          fi
	          sudo sh -ec "echo 1 > /sys/module/friend_loader/parameters/boot"
	          ;;
	      "stop" )
	          if [ `lsmod | grep friend_loader | wc -l` -ge 1 ]; then
		            sudo sh -ec "echo 0 > /sys/module/friend_loader/parameters/boot"
	          fi
	          ;;
	      "restart" )
	          if [ `lsmod | grep friend_loader | wc -l` -ge 1 ]; then
		            sudo sh -ec "echo 0 > /sys/module/friend_loader/parameters/boot"
	          else
		            sudo insmod friend_loader.ko
	          fi
	          sudo sh -ec "echo 1 > /sys/module/friend_loader/parameters/boot"
	          ;;
	      "unload" )
	          if [ `lsmod | grep friend_loader | wc -l` -ge 1 ]; then
		            sudo sh -ec "echo 0 > /sys/module/friend_loader/parameters/boot"
		            sudo rmmod friend_loader.ko
	          fi
	          ;;
    esac
fi
