TESTS = hakase
MAKE := $(MAKE) -f build_rule.mk

default: test

friend.bin: friend.cc
	g++ -O0 -Wall --std=c++14 -nostdinc -nostdlib $(foreach dir, $(INCLUDE_DIR), -iquote $(dir)) -T friend.ld $^ -o $@

hakase.bin: hakase.cc $(TEST_DIR)/test.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	$(MAKE) friend.bin
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin && $(TEST_DIR)/test_hakase.sh 0 $(shell pwd)/$(test).bin $(shell pwd)/../joshu.bin $(shell pwd)/friend.bin &&) :

clean:
	rm -f *.bin raw_bin.o
