include ../../build_rule.mk

ifeq ($(HOST),)

TESTS = data_size invalid reading_signature rw_small rw_large
TEST_OBJS := $(addsuffix .o,$(TESTS)) $(TEST_DIR)test.o
OBJS := $(TEST_OBJS)
DEPS := $(OBJS:%.o=%.d)
.DEFAULT_GOAL:=test

-include $(DEPS)

.PHONY: test

%.bin: %.o $(TEST_DIR)test.o
	g++ $(CXXFLAGS) -o $@ $^

test:
	$(MAKE) -C $(ROOT_DIR) all
	$(MAKE) load
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin && $(TEST_DIR)test_hakase.sh 0 $(shell pwd)/$(test).bin &&) :
	$(MAKE) unload

clean:
	rm -f *.bin $(OBJS) $(DEPS)

endif
