TESTS:=exec
LIBRARIES:=elf_loader
ARGUMENTS=$(QEMU_DIR)$(RELATIVE_DIR)/friend.elf
EX_DEPLOY_FILES:=friend.elf
TMP_FILES:=friend.elf exec.cc *.o app1 app2
include ../../../hakase/test_build_rule.mk 

ifeq ($(HOST),)

exec.cc: ../../common/exec.cc
	cp $^ $@

app1: app1.cc
	g++ --std=c++14 -mcmodel=large -nostdinc -nostdlib -T app.ld $^ -o $@

app1.o: app1
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

app2: app2.cc
	g++ --std=c++14 -mcmodel=large -nostdinc -nostdlib -T app.ld $^ -o $@

app2.o: app2
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

friend.elf: friend.cc ../../common/common.cc app1.o app2.o
	g++ -iquote $(ROOT_DIR)../sample/common $(TEST_CXX_FLAGS) $^ -o $@

endif
