include ../../build_rule.mk

ifeq ($(HOST),)

TESTS = hakase
TEST_OBJS := hakase.o $(TEST_DIR)test.o
OBJS := $(TEST_OBJS)
DEPS := $(OBJS:%.o=%.d)
FRIENDOS_OBJS := friend.o int.o int_asm.o
FRIENDOS_DEPS := $(OBJS:%.o=%.d)
.DEFAULT_GOAL:=test

.PHONY: test

friend.bin: friend.cc
	g++ -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote $(ROOT_DIR) -T friend.ld $^ -o $@

hakase.bin: $(TEST_OBJS) $(MODULE_DIR)libinterrupt.a $(MODULE_DIR)libelf_loader.a
	g++ $(CXXFLAGS) -o $@ $^

test:
	$(MAKE) -C $(ROOT_DIR) all
	$(MAKE) load
	$(MAKE) friend.bin
	$(MAKE) -C $(ROOT_DIR)joshu
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin && $(TEST_DIR)test_hakase.sh 0 $(shell pwd)/$(test).bin $(MODULE_DIR)joshu.bin $(shell pwd)/friend.bin &&) :
	$(MAKE) unload

clean:
	rm -f *.bin $(OBJS) $(DEPS) $(FRIENDOS_OBJS) $(FRIENDOS_DEPS)

endif
