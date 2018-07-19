TESTS:=hakase
LIBRARIES:=elf_loader
ARGUMENTS=$(QEMU_DIR)$(RELATIVE_DIR)/friend.elf
EX_DEPLOY_FILES:=friend.elf
TMP_FILES:=friend.elf
include ../../test_build_rule.mk

ifeq ($(HOST),)

friend.elf: friend.cc
	g++ $(TEST_CXX_FLAGS) $^ -o $@

endif
