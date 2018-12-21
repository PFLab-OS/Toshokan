DOCKER_CMD=docker run --rm -v $(CURDIR):/workdir -w /workdir
HAKASE_CFLAGS:=-g -O0 -MMD -MP -Wall --std=c++14 -static -iquote /workdir/hakase -I /workdir/hakase -iquote /workdir -D __HAKASE__
FRIEND_CFLAGS:=-g -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote /workdir/friend/ -I /workdir/friend/ -iquote /workdir -iquote /workdir/hakase -D__FRIEND__ -T /workdir/friend/friend.ld
CONTAINER_TAG:=3a5eabf0fab92b8099129a3185f5fc98808ec8f3
BUILD_CONTAINER:=livadk/toshokan_build:$(CONTAINER_TAG)
KERNEL_SRC_CONTAINER:=livadk/toshokan_qemu_kernel:$(CONTAINER_TAG)
COMMON_TEST_CFLAGS:=--std=c++14 --coverage -iquote /workdir/common/tests/mock -I /workdir/common/tests/mock -iquote /workdir/ -iquote /workdir/hakase/ -I/cpputest/include -L/cpputest/lib -lCppUTest -lCppUTestExt -pthread
COMMON_TEST_CONTAINER:=livadk/cpputest:aac118d572d3c41bc9e3bed32c7ae8c19249784c

SSH_COMMAND:=$(DOCKER_CMD) -it --network toshokan_net livadk/toshokan_ssh:$(CONTAINER_TAG) ssh -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -p 2222 hakase@toshokan_qemu
SFTP_COMMAND:=$(DOCKER_CMD) -i --network toshokan_net livadk/toshokan_ssh:$(CONTAINER_TAG) sftp -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -P 2222 hakase@toshokan_qemu
REMOTE_USER:=$(if $(ENV_FILE),$(shell . $(ROOT_DIR)/$(ENV_FILE); echo $$SSH_USER),hakase)
#SSH_COMMAND:=ssh nuc
#SFTP_COMMAND:=sftp nuc
#REMOTE_USER:=liva

QEMU_DIR:=/home/$(REMOTE_USER)/share
.DEFAULT_GOAL:all
.PHONY: 

all: hakase/print/print.o hakase/simple_loader/simple_loader.o hakase/elf_loader/elf_loader.o hakase/interrupt/interrupt.o

test: common_test hakase_test

HAKASE_TEST_BIN:= hakase/callback/test/callback.bin hakase/print/test/print.bin hakase/memrw/test/reading_signature.bin hakase/memrw/test/rw_small.bin hakase/memrw/test/rw_large.bin hakase/elf_loader/test/elf_loader.bin hakase/elf_loader/test/friend.elf
hakase_test: $(HAKASE_TEST_BIN) hakase/FriendLoader/friend_loader.ko
	docker rm -f toshokan_qemu || :
	docker network rm toshokan_net || :
	script/build_container.sh $(CONTAINER_TAG)
	docker network create --driver bridge toshokan_net
	$(DOCKER_CMD) -d --name toshokan_qemu --network toshokan_net -P toshokan_qemu_back
	script/transfer.sh "$(SFTP_COMMAND)" $(HAKASE_TEST_BIN)
	$(SSH_COMMAND) $(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)/callback.bin
	$(SSH_COMMAND) $(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)/print.bin
	$(SSH_COMMAND) $(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)/reading_signature.bin
	$(SSH_COMMAND) $(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)/rw_small.bin
	$(SSH_COMMAND) $(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)/rw_large.bin
	$(SSH_COMMAND) $(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)/elf_loader.bin $(QEMU_DIR)/friend.elf
	docker rm -f toshokan_qemu

format:
	@echo "Formatting with clang-format. Please wait..."
	@$(call docker_wrapper,$(FORMAT_CONTAINER_NAME),$(FORMAT_CONTAINER),\
	 git ls-files .. \
	  | grep -E '.*\.cc$$|.*\.h$$' \
		| xargs -n 1 clang-format -style='{BasedOnStyle: Google}' -i \
	 $(if $(CI),&& git diff && git diff | wc -l | xargs test 0 -eq))
	@echo "Done."

tmp: exec.bin tmp.elf hakase/FriendLoader/friend_loader.ko
	docker rm -f toshokan_qemu || :
	docker network rm toshokan_net || :
	script/build_container.sh $(CONTAINER_TAG)
	docker network create --driver bridge toshokan_net
	$(DOCKER_CMD) -d --name toshokan_qemu --network toshokan_net -P toshokan_qemu_back
	script/transfer.sh "$(SFTP_COMMAND)" exec.bin tmp.elf
	#$(SSH_COMMAND) sudo rmmod friend_loader.ko
	sh -c "trap 'docker rm -f toshokan_qemu' EXIT; $(SSH_COMMAND) $(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)/exec.bin $(QEMU_DIR)/tmp.elf"

monitor:
	$(DOCKER_CMD) -it --network toshokan_net livadk/toshokan_ssh:$(CONTAINER_TAG) nc toshokan_qemu 4445

ssh:
	$(SSH_COMMAND)

qemu:
	docker rm -f toshokan_qemu || :
	docker network rm toshokan_net || :
	script/build_container.sh $(CONTAINER_TAG)
	docker network create --driver bridge toshokan_net
	$(DOCKER_CMD) -d --name toshokan_qemu --network toshokan_net -P toshokan_qemu_back
	sh -c "trap 'docker rm -f toshokan_qemu' EXIT; $(SSH_COMMAND)"

TRAMPOLINE_FLAGS:=-Os --std=c++14 -nostdinc -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -iquote friend -iquote hakase -iquote . -D__FRIEND__
TRAMPOLINE_LDFLAGS:=-Os -nostdlib
FRIENDLOADER_DIR:=hakase/FriendLoader
TRAMPOLINE_DIR:=$(FRIENDLOADER_DIR)/trampoline

$(FRIENDLOADER_DIR)/friend_loader.ko: $(FRIENDLOADER_DIR)/*.h $(FRIENDLOADER_DIR)/*.c $(TRAMPOLINE_DIR)/bootentry.S $(TRAMPOLINE_DIR)/main.cc
	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) gcc -Os -c -o $(TRAMPOLINE_DIR)/bootentry.o $(TRAMPOLINE_DIR)/bootentry.S
	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) g++ $(TRAMPOLINE_FLAGS) -c -o $(TRAMPOLINE_DIR)/main.o $(TRAMPOLINE_DIR)/main.cc
	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) ld $(TRAMPOLINE_LDFLAGS) -T $(TRAMPOLINE_DIR)/boot_trampoline.ld -o $(TRAMPOLINE_DIR)/boot_trampoline.bin $(TRAMPOLINE_DIR)/bootentry.o $(TRAMPOLINE_DIR)/main.o
	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) sh -c "cd $(TRAMPOLINE_DIR); objcopy -I binary -O elf64-x86-64 -B i386:x86-64 boot_trampoline.bin bin.o"
	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) script/check_trampoline_bin_size.sh $(TRAMPOLINE_DIR)/bin.o
	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) sh -c "cd $(FRIENDLOADER_DIR); KERN_VERSION=4.13.0-45-generic make all"

{% for rule in rules %}
{{ rule }}
{% endfor %}

DEPENDS_HAKASE_ELFLOADER_TEST:= hakase/tests/test.o hakase/elf_loader/elf_loader.o hakase/elf_loader/test/hakase.cc
hakase/elf_loader/test/elf_loader.bin:$(DEPENDS_HAKASE_ELFLOADER_TEST)
	@echo "CC: (docker)/hakase/elf_loader/test : hakase/elf_loader/test/elf_loader.bin <=$(DEPENDS_HAKASE_ELFLOADER_TEST)"
	@$(DOCKER_CMD) $(BUILD_CONTAINER) g++ $(HAKASE_CFLAGS) -o /workdir/hakase/elf_loader/test/elf_loader.bin $(addprefix /workdir/,$(DEPENDS_HAKASE_ELFLOADER_TEST))

DEPENDS_HAKASE_ELFLOADER_TEST_BIN:= hakase/elf_loader/test/friend.cc
hakase/elf_loader/test/friend.elf:$(DEPENDS_HAKASE_ELFLOADER_TEST_BIN)
	@echo "CC: (docker)/hakase/elf_loader/test : hakase/elf_loader/test/friend.elf <=$(DEPENDS_HAKASE_ELFLOADER_TEST_BIN)"
	$(DOCKER_CMD) $(BUILD_CONTAINER) g++ -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote friend/ -I friend/ -iquote hakase/ -iquote . -D__FRIEND__ -T friend/friend.ld $(DEPENDS_HAKASE_ELFLOADER_TEST_BIN) -o hakase/elf_loader/test/friend.elf

DEPENDS_EXEC:= hakase/tests/test.o hakase/elf_loader/elf_loader.o exec.cc
exec.bin:$(DEPENDS_EXEC)
	@echo "CC: (docker)/exec : exec.bin <=$(DEPENDS_EXEC)"
	@$(DOCKER_CMD) $(BUILD_CONTAINER) g++ $(HAKASE_CFLAGS) -o /workdir/exec.bin $(addprefix /workdir/,$(DEPENDS_EXEC))

DEPENDS_TMP:= sample/common/common.cc int.cc int_asm.S tmp.cc
tmp.elf:$(DEPENDS_TMP)
	@echo "CC: (docker)/ : tmp.elf <=$(DEPENDS_TMP)"
	$(DOCKER_CMD) $(BUILD_CONTAINER) g++ -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote sample/common -iquote friend/ -I friend/ -iquote hakase/ -iquote . -D__FRIEND__ -T friend/friend.ld $(DEPENDS_TMP) -o tmp.elf

run_build_container:
	$(DOCKER_CMD) -it $(BUILD_CONTAINER) sh

clean:
	rm -f {% for file in clean_targets %} {{ file }}{% endfor %} $(FRIENDLOADER_DIR)/*.o $(TRAMPOLINE_DIR)/*.o $(TRAMPOLINE_DIR)/boot_trampoline.bin

%.o : %.cc
