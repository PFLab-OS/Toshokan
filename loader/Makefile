CXXFLAGS = -g -O0 -Wall --std=c++14
TESTS = exec

default: test

raw.bin: raw.c start_raw.S
	gcc -O0 -Wall -fpie -nostdinc -nostdlib -T raw.ld $^ -o $@

raw_bin.o: raw.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

exec.bin: exec.cc raw_bin.o ../test.cc
	g++ -iquote .. $(CXXFLAGS) $^ -o $@

test:
	@$(foreach test, $(TESTS), make $(test).bin; ../test_hakase.sh 0 $(shell pwd)/$(test).bin; )

clean:
	-rm exec *.bin raw_bin.o
