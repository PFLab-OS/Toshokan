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

QEMU_OPTION:=-cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic -global hpet.msi=true
TOSHOKAN_CONTAINER_IMAGE:=livadk/toshokan_qemu
TOSHOKAN_QEMU_HOST:=toshokan_qemu

ifeq ($(DEBUG),1)
DOCKER_OPTION+=--cap-add=SYS_PTRACE
TOSHOKAN_CONTAINER_IMAGE:=$(TOSHOKAN_CONTAINER_IMAGE)_debug
endif

define CALL_QEMU
	$(call SILENT_EXEC,docker exec -it $(TOSHOKAN_CONTAINER) $1,$2)
endef

FRIEND_CXXFLAGS:=-O2 -Wall -Werror=unused-result --std=c++14 -mcmodel=large -nostdinc -nostdlib -fno-pie -no-pie -D__FRIEND__ -T /usr/local/etc/friend.ld -I/usr/local/include -L/usr/local/lib64 -lfriend -lcommon
HAKASE_CXXFLAGS:=-O0 -Wall -Werror=unused-result --std=c++14 -mcmodel=large -static -fno-pie -no-pie -D__HAKASE__ -T /usr/local/etc/hakase.ld -lhakase -lcommon

DEFAULT: run

friend.bin: $(FRIEND_SOURCE)
	$(call SILENT_EXEC,.misc/wrapper/friend-g++ $(filter-out %.h,$(FRIEND_SOURCE)) $(FRIEND_CXXFLAGS) -o $@,building friend binary)

friend_bin.o: friend.bin
	$(call SILENT_EXEC,.misc/wrapper/objcopy -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=friend_bin $^ $@,converting friend binary to embedded format)

friend.sym: friend.bin
	$(call SILENT_EXEC,.misc/wrapper/objcopy --prefix-symbols=friendsymbol_ $^ $@,generating symbol tables)

hakase.bin: $(HAKASE_SOURCE) friend_bin.o friend.sym
	$(call SILENT_EXEC,.misc/wrapper/hakase-g++ $(filter-out %.h,$(HAKASE_SOURCE)) friend_bin.o $(HAKASE_CXXFLAGS) -o $@ -Wl$(comma)-R$(comma)friend.sym,building hakase binary \& combining friend binary with it)
	$(call SILENT_EXEC,.misc/wrapper/strip --strip-debug $@,stripping debug info)

.PHONY: prepare_qemu wait_qemu qemu_run remote_run run clean

.FORCE:

prepare_qemu: .FORCE
	$(call SILENT_EXEC,docker rm -f $(TOSHOKAN_CONTAINER) > /dev/null 2>&1 || :,cleaning up old environments)
	$(call SILENT_EXEC,docker run -d $(DOCKER_OPTION) --name $(TOSHOKAN_CONTAINER) -v $(CURDIR):$(CURDIR) -w $(CURDIR) $(TOSHOKAN_CONTAINER_IMAGE):$(TOSHOKAN_VERSION) qemu-system-x86_64 $(QEMU_OPTION) > /dev/null 2>&1,starting a QEMU container)

wait_qemu: prepare_qemu
	$(call SILENT_EXEC,docker exec -i $(TOSHOKAN_CONTAINER) wait-for-rsync $(TOSHOKAN_QEMU_HOST),waiting until the QEMU container is ready)

qemu_run: prepare_qemu hakase.bin wait_qemu
	$(call CALL_QEMU,rsync -z hakase.bin $(TOSHOKAN_QEMU_HOST):,sending the binary to remote)
	$(call CALL_QEMU,ssh $(TOSHOKAN_QEMU_HOST) sudo ./hakase.bin,running hakase.bin on remote)
	$(call SILENT_EXEC,docker rm -f $(TOSHOKAN_CONTAINER) > /dev/null 2>&1,cleaning up the environment)
	$(call SILENT_EXEC,cp hakase.bin hakase.phys.bin)

ifdef HOST
run: remote_run

hakase.phys.bin: qemu_run

remote_run: hakase.phys.bin
	$(call SILENT_EXEC,rsync -z hakase.phys.bin $(HOST):,sending the binary to remote)
	$(call SILENT_EXEC,ssh $(HOST) sudo ./hakase.phys.bin,running hakase.bin on remote)
else
run: qemu_run
endif

clean:
	$(call SILENT_EXEC,rm -rf friend.bin friend_bin.o friend.sym hakase.bin hakase.phys.bin,deleting all intermediate files)

monitor:
	$(call SILENT_EXEC,docker exec $(TOSHOKAN_CONTAINER) sh -c "echo 'cpu 1' | busybox nc localhost 4445 > /dev/zero")
	$(call SILENT_EXEC,docker exec -it $(TOSHOKAN_CONTAINER) busybox nc localhost 4445,connecting to QEMU monitor)

debug_qemu:
	$(call CALL_QEMU,gdb -p 1,debugging qemu with gdb)

attach_gdb:
	$(call CALL_QEMU,gdb -ex "target remote localhost:1234" -ex "thread 2",attaching gdb to qemu)
