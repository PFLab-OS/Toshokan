TESTS:=exec
LIBRARIES:=elf_loader
ARGUMENTS=$(QEMU_DIR)$(RELATIVE_DIR)/friend.elf
EX_DEPLOY_FILES:=friend.elf
TMP_FILES:=friend.elf exec.cc
include ../../../hakase/test_build_rule.mk 

ifeq ($(HOST),)

exec.cc: ../../common/exec.cc
	cp $^ $@

friend.elf: friend.cc
	g++ $(TEST_CXX_FLAGS) $^ -o $@

endif
