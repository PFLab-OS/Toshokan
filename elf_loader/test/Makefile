include ../../build_rule.mk

ifeq ($(HOST),)

TESTS = hakase
.DEFAULT_GOAL:=test

.PHONY: test

friend.bin: friend.cc
	g++ -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote $(ROOT_DIR) -T friend.ld $^ -o $@

hakase.bin: hakase.cc $(TEST_DIR)/test.cc ../hakase.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	$(MAKE) load
	$(MAKE) friend.bin
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin && $(TEST_DIR)test_hakase.sh 0 $(shell pwd)/$(test).bin $(shell pwd)/friend.bin &&) :
	$(MAKE) unload

clean:
	rm -f *.bin

endif
