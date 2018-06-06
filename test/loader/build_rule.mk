TESTS = exec
MAKE := $(MAKE) -f build_rule.mk

default: test

raw.bin: raw.cc
	g++ -O0 -Wall --std=c++14 -fpie -nostdinc -nostdlib -iquote $(INCLUDE_DIR) -T raw.ld $^ -o $@

exec.bin: exec.cc ../test.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin; ../test_hakase.sh 0 $(shell pwd)/$(test).bin $(shell pwd)/raw.bin; )

clean:
	rm -f *.bin raw_bin.o
