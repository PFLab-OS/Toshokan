BOOTOBJS := bootentry.o main.o

FLAGS := -Os -nostdinc -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables
CXXFLAGS := $(FLAGS) --std=c++14
CFLAGS := $(FLAGS)
ASFLAGS := -Os
LDFLAGS := -Os -nostdlib

default: bin.o

bin.o: boot_trampoline.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

boot_trampoline.bin: $(BOOTOBJS)
	ld $(LDFLAGS) -Tboot_trampoline.ld $^ -o $@

clean:
	-rm *.o boot_trampoline.bin
