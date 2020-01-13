ifdef SIGNATURE
SIGNATURE:=_$(SIGNATURE)
endif

TOSHOKAN_CONTAINER:=toshokan_qemu$(SIGNATURE)

ifeq ($(V),1)
define SILENT_EXEC
	$1
endef
else
$(info Silent mode is enabled by default. You can turn it off with 'make V=1'.)
define SILENT_EXEC
	@$(if $2,echo ">>>" $2)
	@$1
endef
endif

comma:=,

# if HOST is not defined, run on docker container
ifndef HOST
HOST:=toshokan_qemu
define CALL_REMOTE
	$(call SILENT_EXEC,docker exec -it $(TOSHOKAN_CONTAINER) $1,$2)
endef
QEMU_OPTION:=-cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic -global hpet.msi=true
TOSHOKAN_CONTAINER_IMAGE:=livadk/toshokan_qemu
ifeq ($(DEBUGQEMU),1)
DOCKER_OPTION+=--cap-add=SYS_PTRACE
TOSHOKAN_CONTAINER_IMAGE:=$(TOSHOKAN_CONTAINER_IMAGE)_debug
endif

define PREPARE_REMOTE
	$(call SILENT_EXEC,docker rm -f $(TOSHOKAN_CONTAINER) > /dev/null 2>&1 || :,clean up old environments)
	$(call SILENT_EXEC,docker run -d $(DOCKER_OPTION) --name $(TOSHOKAN_CONTAINER) -v $(CURDIR):$(CURDIR) -w $(CURDIR) $(TOSHOKAN_CONTAINER_IMAGE):$(TOSHOKAN_VERSION) qemu-system-x86_64 $(QEMU_OPTION) > /dev/null 2>&1,start a QEMU container)
endef

define WAIT_REMOTE
	$(call SILENT_EXEC,docker exec -i $(TOSHOKAN_CONTAINER) wait-for-rsync $(HOST),wait until the QEMU container is ready)
endef

define CLEANUP_REMOTE
	$(call SILENT_EXEC,docker rm -f $(TOSHOKAN_CONTAINER) > /dev/null 2>&1,clean up the environment)
endef
else
define CALL_REMOTE
	$(call SILENT_EXEC,$1,$2)
endef
endif

FRIEND_CXXFLAGS:=-g -O0 -Wall -Werror=unused-result --std=c++14 -nostdinc -nostdlib -fno-pie -no-pie -D__FRIEND__ -T /usr/local/etc/friend.ld -I/usr/local/include -L/usr/local/lib64 -lfriend -lcommon
HAKASE_CXXFLAGS:=-g -O0 -Wall -Werror=unused-result --std=c++14 -static -fno-pie -no-pie -D__HAKASE__ -T /usr/local/etc/hakase.ld -lhakase -lcommon

DEFAULT: run

friend.bin: $(FRIEND_SOURCE)
	$(call SILENT_EXEC,.misc/wrapper/friend-g++ $(filter-out %.h,$(FRIEND_SOURCE)) $(FRIEND_CXXFLAGS) -o $@,build friend binary)

friend_bin.o: friend.bin
	$(call SILENT_EXEC,.misc/wrapper/objcopy -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=friend_bin $^ $@,convert friend binary to embedded format)

friend.sym: friend.bin
	$(call SILENT_EXEC,.misc/wrapper/objcopy --prefix-symbols=friendsymbol_ $^ $@,generate symbol tables)

hakase.bin: $(HAKASE_SOURCE) friend_bin.o friend.sym
	$(call SILENT_EXEC,.misc/wrapper/hakase-g++ $(filter-out %.h,$(HAKASE_SOURCE)) friend_bin.o $(HAKASE_CXXFLAGS) -o $@ -Wl$(comma)-R$(comma)friend.sym,build hakase binary \& combine friend binary with it)

.PHONY: prepare_remote wait_remote run clean

.FORCE:

prepare_remote: .FORCE
	$(PREPARE_REMOTE)

wait_remote: prepare_remote
	$(WAIT_REMOTE)

run: prepare_remote hakase.bin wait_remote
	$(call CALL_REMOTE,rsync -z hakase.bin $(HOST):,send the binary to remote)
	$(call CALL_REMOTE,ssh $(HOST) sudo ./hakase.bin,run hakase.bin on remote)
	$(CLEANUP_REMOTE)

clean:
	$(call SILENT_EXEC,rm -rf friend.bin friend_bin.o friend.sym hakase.bin,delete all intermediate files)

monitor:
	$(call SILENT_EXEC,docker exec $(TOSHOKAN_CONTAINER) sh -c "echo 'cpu 1' | busybox nc localhost 4445 > /dev/zero")
	$(call SILENT_EXEC,docker exec -it $(TOSHOKAN_CONTAINER) busybox nc localhost 4445,connecting to QEMU monitor)
