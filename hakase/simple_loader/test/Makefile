TESTS:=hakase
LIBRARIES:=simple_loader
ARGUMENTS=$(QEMU_DIR)$(RELATIVE_DIR)/raw.elf
EX_DEPLOY_FILES:=raw.elf
TMP_FILES:=raw.elf
include ../../test_build_rule.mk

ifeq ($(HOST),)

raw.elf: raw.cc
	g++ $(RAW_CXX_FLAGS) -T raw.ld $^ -o $@

endif
