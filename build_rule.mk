.PHONY: qemu qemu-telnet

qemu:
	bash sync/VirtualBox_VM/run_arm.sh qemu

qemu-telnet:
	bash sync/VirtualBox_VM/run_arm.sh telnet
