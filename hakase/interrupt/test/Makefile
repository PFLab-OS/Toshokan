TESTS:=hakase
LIBRARIES:=interrupt elf_loader
ARGUMENTS=$(QEMU_DIR)$(RELATIVE_DIR)/friend.elf
EX_DEPLOY_FILES:=friend.elf
TMP_FILES:=friend.elf
include ../../test_build_rule.mk

ifeq ($(HOST),)

friend.elf: friend.cc int.cc int_asm.S
	g++ -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote $(ROOT_DIR) -D __FRIEND__ -T friend.ld $^ -o $@

endif
