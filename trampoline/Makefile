OBJS := entry.o

CXXFLAGS := -Os -nostdinc -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -fpic -fPIC
LDFLAGS := -Os -nostdlib -pie

default: bin.o

bin.o: trampoline.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

trampoline.bin: $(OBJS)
	ld $(LDFLAGS) -Ttrampoline.ld $^ -o $@

clean:
	-rm *.o trampoline.bin
