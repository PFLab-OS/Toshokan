.PHONY: qemu arm-sync

qemu:
	bash sync/qemu.sh

arm-sync:
	rsync -avhP -r --delete /home/vagrant/sync arm:~
