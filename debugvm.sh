#!/bin/bash
# example
# ./debugvm.sh getregisters --cpu=1 rip

VBoxManage debugvm $(cat $(vagrant status | grep "host_dir:" | cut -f2)/.vagrant/machines/default/virtualbox/id) $@ 
