include ../../build_rule.mk

ifeq ($(HOST),)

TESTS = hakase
TEST_OBJS := hakase.o $(TEST_DIR)test.o
OBJS := $(TEST_OBJS)
DEPS := $(OBJS:%.o=%.d)
.DEFAULT_GOAL:=test

-include $(DEPS)

.PHONY: test

raw.bin: raw.cc
	g++ -O0 -Wall --std=c++14 -fpie -nostdinc -nostdlib -iquote $(ROOT_DIR) -T raw.ld $^ -o $@

hakase.bin: $(TEST_OBJS)
	g++ $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

test:
	$(MAKE) -C $(ROOT_DIR) all
	$(MAKE) load
	$(MAKE) raw.bin
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin && $(TEST_DIR)test_hakase.sh 0 $(shell pwd)/$(test).bin $(shell pwd)/raw.bin &&) :
	$(MAKE) unload

clean:
	rm -f *.bin $(OBJS) $(DEPS)

endif
