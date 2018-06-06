TESTS = hakase
MAKE := $(MAKE) -f build_rule.mk

default: test

raw.bin: raw.cc
	g++ -O0 -Wall --std=c++14 -fpie -nostdinc -nostdlib  $(foreach dir, $(INCLUDE_DIR), -iquote $(dir)) -T raw.ld $^ -o $@

hakase.bin: hakase.cc ../test.cc ../../simple_loader/hakase.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin; ../test_hakase.sh 0 $(shell pwd)/$(test).bin $(shell pwd)/raw.bin; )

clean:
	rm -f *.bin raw_bin.o
