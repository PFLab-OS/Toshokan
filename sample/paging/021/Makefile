TESTS:=exec
LIBRARIES:=elf_loader
ARGUMENTS=$(QEMU_DIR)$(RELATIVE_DIR)/friend.elf
EX_DEPLOY_FILES:=friend.elf
TMP_FILES:=friend.elf *.o app1
include ../../../hakase/test_build_rule.mk 

ifeq ($(HOST),)

app1: app1.cc
	g++ --std=c++14 -nostdinc -nostdlib -T app1.ld $^ -o $@

app1.o: app1
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

friend.elf: friend.cc ../../common/common.cc app1.o
	g++ -iquote $(ROOT_DIR)../sample/common $(TEST_CXX_FLAGS) $^ -o $@

endif
