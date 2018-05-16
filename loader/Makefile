CXXFLAGS = -g -O0 -Wall --std=c++14

default: test

exec: exec.cc raw_bin.o
	g++ -iquote .. $(CXXFLAGS) $^ -o $@

raw.bin: raw.c start_raw.S
	gcc -O0 -Wall -fpie -nostdinc -nostdlib -T raw.ld $^ -o $@

raw_bin.o: raw.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

test:
	make exec; cd ..; ./test.sh ./loader/exec

clean:
	-rm exec raw.bin raw_bin.o
