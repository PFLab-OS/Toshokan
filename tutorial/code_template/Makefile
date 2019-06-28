ifdef SIGNATURE
SIGNATURE := _$(SIGNATURE)
endif

CONTAINER_NETWORK := toshokan_net$(SIGNATURE)
TOSHOKAN_CONTAINER := toshokan_qemu$(SIGNATURE)
TAG_VERSION := v0.04a

# if HOST is not defined, run on docker container
ifndef HOST
HOST := toshokan_qemu
CALL_REMOTE := docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh:$(TAG_VERSION)
QEMU_OPTION := -cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic
define PREPARE_REMOTE
	docker rm -f $(TOSHOKAN_CONTAINER) > /dev/null 2>&1 || :
	docker network rm $(CONTAINER_NETWORK) > /dev/null 2>&1 || :
	docker network create --driver bridge $(CONTAINER_NETWORK)
	docker run -d --name $(TOSHOKAN_CONTAINER) --network $(CONTAINER_NETWORK) --net-alias $(HOST) livadk/toshokan_qemu:$(TAG_VERSION) qemu-system-x86_64 $(QEMU_OPTION)
	docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh:$(TAG_VERSION) wait-for-rsync $(HOST)
endef

define CLEANUP_REMOTE
	docker rm -f $(TOSHOKAN_CONTAINER)
	docker network rm $(CONTAINER_NETWORK)
endef
endif

FRIEND_CXXFLAGS := -g -O0 -Wall -Werror=unused-result --std=c++14 -nostdinc -nostdlib -fno-pie -no-pie -D__FRIEND__ -T /usr/local/etc/friend.ld -I/usr/local/include -L/usr/local/lib64 -lfriend -lcommon
HAKASE_CXXFLAGS := -g -O0 -Wall -Werror=unused-result --std=c++14 -static -fno-pie -no-pie -D__HAKASE__ -T /usr/local/etc/hakase.ld -lhakase -lcommon

DEFAULT: run

friend.bin: shared.h friend.cc
	bin/friend-g++ friend.cc $(FRIEND_CXXFLAGS) -o $@

friend_bin.o: friend.bin
	bin/objcopy -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=friend_bin $^ $@

friend.sym: friend.bin
	bin/objcopy --prefix-symbols=friendsymbol_ $^ $@

hakase.bin: shared.h hakase.cc friend_bin.o friend.sym
	bin/hakase-g++ friend_bin.o hakase.cc $(HAKASE_CXXFLAGS) -o $@ -Wl,-R,friend.sym

run: hakase.bin
	$(PREPARE_REMOTE)
	$(CALL_REMOTE) rsync $^ $(HOST):
	$(CALL_REMOTE) ssh $(HOST) sudo ./$^
	$(CLEANUP_REMOTE)

clean:
	-rm -rf friend.bin friend_bin.o friend.sym hakase.bin

monitor:
	docker exec -it $(TOSHOKAN_CONTAINER) nc localhost 4445
