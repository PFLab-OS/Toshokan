include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))build_rule.mk

ifeq ($(HOST),)

OBJS:=$(addsuffix .o,$(TESTS)) $(TEST_DIR)test.o
DEPS:=$(OBJS:%.o=%.d)
TEST_BINS:=$(addsuffix .bin, $(TESTS))
DEPLOY_FILES:=$(TEST_BINS) $(EX_DEPLOY_FILES)
TEST_CXX_FLAGS:=-g -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote $(ROOT_DIR) -D__FRIEND__
.DEFAULT_GOAL:=test

-include $(DEPS)

.PHONY: FORCE test bin clean pre-deploy
.PRECIOUS: %.o

FORCE:

%.bin: %.o $(TEST_DIR)test.o $(foreach lib, $(LIBRARIES), $(MODULE_DIR)lib$(lib).a)
	g++ $(CXXFLAGS) -o $@ $^

test: FORCE
	$(MAKE) -C $(ROOT_DIR) prepare_test
	$(MAKE) pre-deploy
	$(MAKE) -C $(ROOT_DIR) execute_test

bin: $(TEST_BINS) $(DEPLOY_FILES)

clean: FORCE
	rm -f *.bin $(OBJS) $(DEPS) $(TMP_FILES)

pre-deploy: FORCE
	$(MAKE) bin
	mkdir -p $(DEPLOY_DIR)$(RELATIVE_DIR)/
	$(foreach file, $(DEPLOY_FILES), cp $(file) $(DEPLOY_DIR)$(RELATIVE_DIR)/$(file) &&) :
	$(foreach file, $(TEST_BINS), echo "$(QEMU_DIR)/test_hakase.sh $(QEMU_DIR)$(RELATIVE_DIR)/$(file) $(ARGUMENTS)" >> $(RUN_SCRIPT) &&) :

endif
