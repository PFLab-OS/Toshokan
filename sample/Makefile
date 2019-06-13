ifdef SIGNATURE
SIGNATURE := _$(SIGNATURE)
endif

CONTAINER_NETWORK := toshokan_net$(SIGNATURE)
TOSHOKAN_CONTAINER := toshokan_net$(SIGNATURE)
QEMU_OPTION := -cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic

FRIEND_CXXFLAGS := -g -O0 -Wall -Werror=unused-result --std=c++14 -nostdinc -nostdlib -fno-pie -no-pie -D__FRIEND__ -T /usr/local/etc/friend.ld -I/usr/local/include -L/usr/local/lib64 -lfriend -lcommon
HAKASE_CXXFLAGS := -g -O0 -Wall -Werror=unused-result --std=c++14 -static -fno-pie -no-pie -D__HAKASE__ -T /usr/local/etc/hakase.ld -lhakase -lcommon

DEFAULT: run

friend.bin: shared.h friend_boot.S friend.cc
	bin/friend-g++ friend_boot.S friend.cc $(FRIEND_CXXFLAGS) -o $@

friend_bin.o: friend.bin
	bin/objcopy -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=friend_bin $^ $@

friend.sym: friend.bin
	bin/objcopy --prefix-symbols=friendsymbol_ $^ $@

hakase.bin: shared.h hakase.cc friend_bin.o friend.sym
	bin/hakase-g++ friend_bin.o hakase.cc $(HAKASE_CXXFLAGS) -o $@ -Wl,-R,friend.sym

run: hakase.bin
	docker rm -f $(TOSHOKAN_CONTAINER) > /dev/null 2>&1 || :
	docker network rm $(CONTAINER_NETWORK) > /dev/null 2>&1 || :
	docker network create --driver bridge $(CONTAINER_NETWORK)
	docker run -d --name $(TOSHOKAN_CONTAINER) --network $(CONTAINER_NETWORK) --net-alias toshokan_qemu livadk/toshokan_qemu:v0.1 qemu-system-x86_64 $(QEMU_OPTION)
	docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh:v0.1 wait-for-rsync toshokan_qemu
	docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh:v0.1 rsync $^ toshokan_qemu:
	docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh:v0.1 ssh toshokan_qemu sudo ./$^
	docker rm -f $(TOSHOKAN_CONTAINER)
	docker network rm $(CONTAINER_NETWORK)

clean:
	-rm -rf friend_bin.o friend.sym hakase.bin
