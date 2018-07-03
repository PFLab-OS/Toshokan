include ../../build_rule.mk

ifeq ($(HOST),)

TESTS = hakase
TEST_OBJS := hakase.o $(TEST_DIR)test.o
OBJS := $(TEST_OBJS)
DEPS := $(OBJS:%.o=%.d)
.DEFAULT_GOAL:=test

-include $(DEPS)

.PHONY: test

hakase.bin: $(TEST_OBJS) $(MODULE_DIR)libprint.a
	g++ $(CXXFLAGS) -o $@ $^

test:
	$(MAKE) -C $(ROOT_DIR) all
	$(MAKE) load
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin && $(TEST_DIR)test_hakase.sh 0 $(shell pwd)/$(test).bin &&) :
	$(MAKE) unload

clean:
	rm -f *.bin $(OBJS) $(DEPS)

endif
