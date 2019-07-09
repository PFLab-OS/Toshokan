ifdef SIGNATURE
SIGNATURE := _$(SIGNATURE)
endif

CONTAINER_NETWORK := toshokan_net$(SIGNATURE)
TOSHOKAN_CONTAINER := toshokan_qemu$(SIGNATURE)
TAG_VERSION := v0.04a

ifeq ($(V),1)
define SILENT_EXEC
	$1
endef
else
$(info Silent mode is enabled by default. You can turn it off with 'make V=1'.)
define SILENT_EXEC
	@$(if $2,echo INFO: $2)
	@$1
endef
endif

comma := ,

# if HOST is not defined, run on docker container
ifndef HOST
HOST := toshokan_qemu
define CALL_REMOTE
	$(call SILENT_EXEC,docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh:$(TAG_VERSION) $1,$2)
endef
QEMU_OPTION := -cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic -global hpet.msi=true
define PREPARE_REMOTE
	$(call SILENT_EXEC,docker rm -f $(TOSHOKAN_CONTAINER) > /dev/null 2>&1 || :,clean up old environments)
	$(call SILENT_EXEC,docker network rm $(CONTAINER_NETWORK) > /dev/null 2>&1 || :,)
	$(call SILENT_EXEC,docker network create --driver bridge $(CONTAINER_NETWORK) > /dev/null 2>&1,create docker network)
	$(call SILENT_EXEC,docker run -d --name $(TOSHOKAN_CONTAINER) --network $(CONTAINER_NETWORK) --net-alias $(HOST) livadk/toshokan_qemu:$(TAG_VERSION) qemu-system-x86_64 $(QEMU_OPTION) > /dev/null 2>&1,start a QEMU container)
	$(call SILENT_EXEC,docker run -it --rm -v $(CURDIR):$(CURDIR) -w $(CURDIR) --network $(CONTAINER_NETWORK) livadk/toshokan_ssh:$(TAG_VERSION) wait-for-rsync $(HOST),wait until the QEMU container is ready)
endef

define CLEANUP_REMOTE
	$(call SILENT_EXEC,docker rm -f $(TOSHOKAN_CONTAINER),clean up the envrionment)
	$(call SILENT_EXEC,docker network rm $(CONTAINER_NETWORK))
endef
else
define CALL_REMOTE
	$(call SILENT_EXEC,$1,$2)
endef
endif

FRIEND_CXXFLAGS := -g -O0 -Wall -Werror=unused-result --std=c++14 -nostdinc -nostdlib -fno-pie -no-pie -D__FRIEND__ -T /usr/local/etc/friend.ld -I/usr/local/include -L/usr/local/lib64 -lfriend -lcommon
HAKASE_CXXFLAGS := -g -O0 -Wall -Werror=unused-result --std=c++14 -static -fno-pie -no-pie -D__HAKASE__ -T /usr/local/etc/hakase.ld -lhakase -lcommon

DEFAULT: run

friend.bin: shared.h friend.cc
	$(call SILENT_EXEC,bin/friend-g++ friend.cc $(FRIEND_CXXFLAGS) -o $@,build friend binary)

friend_bin.o: friend.bin
	$(call SILENT_EXEC,bin/objcopy -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=friend_bin $^ $@,convert friend binary to embedded format)

friend.sym: friend.bin
	$(call SILENT_EXEC,bin/objcopy --prefix-symbols=friendsymbol_ $^ $@,generate symbol tables)

hakase.bin: shared.h hakase.cc friend_bin.o friend.sym
	$(call SILENT_EXEC,bin/hakase-g++ friend_bin.o hakase.cc $(HAKASE_CXXFLAGS) -o $@ -Wl$(comma)-R$(comma)friend.sym,build hakase binary \& combine friend binary with it)

run: hakase.bin
	$(PREPARE_REMOTE)
	$(call CALL_REMOTE,rsync $^ $(HOST):,send the binary to remote)
	$(call CALL_REMOTE,ssh $(HOST) sudo ./$^,run $^ on remote)
	$(CLEANUP_REMOTE)

clean:
	$(call SILENT_EXEC,rm -rf friend.bin friend_bin.o friend.sym hakase.bin,delete all intermediate files)

monitor:
	$(call SILENT_EXEC,docker exec $(TOSHOKAN_CONTAINER) sh -c "echo 'cpu 1' | nc localhost 4445 > /dev/zero")
	$(call SILENT_EXEC,docker exec -it $(TOSHOKAN_CONTAINER) nc localhost 4445,connecting to QEMU monitor)
