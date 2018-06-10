include ../../build_rule.mk

ifeq ($(HOST),)

TESTS = hakase
.DEFAULT_GOAL:=test

.PHONY: test

raw.bin: raw.cc
	g++ -O0 -Wall --std=c++14 -fpie -nostdinc -nostdlib -iquote $(ROOT_DIR) -T raw.ld $^ -o $@

hakase.bin: hakase.cc $(TEST_DIR)test.cc ../hakase.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	$(MAKE) load
	$(MAKE) raw.bin
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin && $(TEST_DIR)test_hakase.sh 0 $(shell pwd)/$(test).bin $(shell pwd)/raw.bin &&) :
	$(MAKE) unload

clean:
	rm -f *.bin

endif
