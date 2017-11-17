.PHONY: qemu qemu-telnet

qemu:
	bash sync/run_arm.sh qemu

qemu-telnet:
	bash sync/run_arm.sh telnet
