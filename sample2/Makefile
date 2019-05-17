ifndef ($(SIGNATURE),)
SIGNATURE := _$(SIGNATURE)
endif

CONTAINER_NETWORK := toshokan_net$(SIGNATURE)
TOSHOKAN_CONTAINER := toshokan_net$(SIGNATURE)
QEMU_OPTION := -cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic

DEFAULT: run

friend.bin: friend_boot.S friend.cc
	bin/friend-g++ -g -O0 -Wall --std=c++14 -nostdinc -nostdlib -I/usr/local/include -L/usr/local/lib64  -fno-pie -no-pie -T /usr/local/etc/friend.ld -o $@ $^ -lfriend

friend_bin.o: friend.bin
	bin/objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

friend.shsym: friend.bin
	bin/objcopy -j .shsym $^ $@

hakase.bin: hakase.cc friend_bin.o friend.shsym
	bin/hakase-g++ -g -O0 -Wall --std=c++14 -static -fno-pie -no-pie -Wl,-R,friend.shsym -o $@ /usr/local/etc/hakase.ld friend_bin.o hakase.cc -lhakase

run: hakase.bin
	docker rm -f $(TOSHOKAN_CONTAINER) > /dev/null 2>&1 || :
	docker network rm $(CONTAINER_NETWORK) > /dev/null 2>&1 || :
	docker network create --driver bridge $(CONTAINER_NETWORK)
	docker run -d --name $(TOSHOKAN_CONTAINER) --network $(CONTAINER_NETWORK) --net-alias toshokan_qemu livadk/toshokan_qemu qemu-system-x86_64 $(QEMU_OPTION)
	docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh wait-for-rsync toshokan_qemu
	docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh rsync $^ toshokan_qemu:
	docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh ssh toshokan_qemu sudo ./test_library.sh ./$^
	docker rm -f $(TOSHOKAN_CONTAINER)
	docker network rm $(CONTAINER_NETWORK)
