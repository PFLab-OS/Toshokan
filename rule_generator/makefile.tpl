REMOTE_USER:=$(if $(ENV_FILE),$(shell . $(ROOT_DIR)/$(ENV_FILE); echo $$SSH_USER),hakase)
QEMU_DIR:=/home/$(REMOTE_USER)/share
DOCKER_CMD=docker run --rm -v $(CURDIR):/workdir -w /workdir
HAKASE_CFLAGS:=-g -O0 -MMD -MP -Wall --std=c++14 -iquote /workdir/hakase -I /workdir/hakase -iquote /workdir -D __HAKASE__
FRIEND_CFLAGS:=-g -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote /workdir/friend/ -I /workdir/friend/ -iquote /workdir -iquote /workdir/hakase -D__FRIEND__ -T /workdir/friend/friend.ld
CONTAINER_TAG:=07343ded0d0090e08f8004f2ee9f7566ef623262
BUILD_CONTAINER:=livadk/toshokan_build:$(CONTAINER_TAG)
SSH_COMMAND:=$(DOCKER_CMD) -i --network toshokan_net livadk/toshokan_ssh:$(CONTAINER_TAG) ssh -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -p 2222 hakase@toshokan_qemu
COMMON_TEST_CFLAGS:=--std=c++14 --coverage -iquote /workdir/common/tests/mock -I /workdir/common/tests/mock -iquote /workdir/ -iquote /workdir/hakase/ -I/cpputest/include -L/cpputest/lib -lCppUTest -lCppUTestExt -pthread
COMMON_TEST_CONTAINER:=livadk/cpputest:aac118d572d3c41bc9e3bed32c7ae8c19249784c
.DEFAULT_GOAL:all
.PHONY: 

all: hakase/print/print.o hakase/simple_loader/simple_loader.o hakase/elf_loader/elf_loader.o hakase/interrupt/interrupt.o

build: hakase/callback/test/callback.bin hakase/FriendLoader/friend_loader.ko
	docker rm -f toshokan_qemu || :
	docker network rm toshokan_net || :
	script/build_container.sh $(CONTAINER_TAG)
	docker network create --driver bridge toshokan_net
	$(DOCKER_CMD) -d --name toshokan_qemu --network toshokan_net --publish 2222:2222 toshokan_qemu_back
	script/transfer.sh $(CONTAINER_TAG) hakase/callback/test/callback.bin
	$(SSH_COMMAND) $(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)/callback.bin
	docker rm -f toshokan_qemu

TRAMPOLINE_FLAGS:=-Os --std=c++14 -nostdinc -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -iquote friend -iquote hakase -iquote . -D__FRIEND__
TRAMPOLINE_LDFLAGS:=-Os -nostdlib
FRIENDLOADER_DIR:=hakase/FriendLoader
TRAMPOLINE_DIR:=$(FRIENDLOADER_DIR)/trampoline

$(FRIENDLOADER_DIR)/friend_loader.ko: $(FRIENDLOADER_DIR)/*.h $(FRIENDLOADER_DIR)/*.c $(TRAMPOLINE_DIR)/bootentry.S $(TRAMPOLINE_DIR)/main.cc
	$(DOCKER_CMD) livadk/toshokan_kernel_build:$(CONTAINER_TAG) gcc -Os -c -o $(TRAMPOLINE_DIR)/bootentry.o $(TRAMPOLINE_DIR)/bootentry.S
	$(DOCKER_CMD) livadk/toshokan_kernel_build:$(CONTAINER_TAG) g++ $(TRAMPOLINE_FLAGS) -c -o $(TRAMPOLINE_DIR)/main.o $(TRAMPOLINE_DIR)/main.cc
	$(DOCKER_CMD) livadk/toshokan_kernel_build:$(CONTAINER_TAG) ld $(TRAMPOLINE_LDFLAGS) -T $(TRAMPOLINE_DIR)/boot_trampoline.ld -o $(TRAMPOLINE_DIR)/boot_trampoline.bin $(TRAMPOLINE_DIR)/bootentry.o $(TRAMPOLINE_DIR)/main.o
	$(DOCKER_CMD) livadk/toshokan_kernel_build:$(CONTAINER_TAG) sh -c "cd $(TRAMPOLINE_DIR); objcopy -I binary -O elf64-x86-64 -B i386:x86-64 boot_trampoline.bin bin.o"
	$(DOCKER_CMD) livadk/toshokan_kernel_build:$(CONTAINER_TAG) script/check_trampoline_bin_size.sh $(TRAMPOLINE_DIR)/bin.o
	$(DOCKER_CMD) livadk/toshokan_kernel_build:$(CONTAINER_TAG) sh -c "cd $(FRIENDLOADER_DIR); make all"

{% for rule in rules %}
{{ rule }}
{% endfor %}

clean:
	rm -f Makefile {% for file in clean_targets %} {{ file }}{% endfor %} $(FRIENDLOADER_DIR)/*.o $(TRAMPOLINE_DIR)/*.o $(TRAMPOLINE_DIR)/boot_trampoline.bin

%.o : %.cc
