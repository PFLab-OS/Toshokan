BOOTOBJS := bootentry.o main.o int_asm.o int.o

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

check_bin_size: $(BOOTOBJS)
	-$(eval tmpf := $(shell mktemp))
	-$(eval tmpbin := $(shell mktemp))
	@cat boot_trampoline.ld | sed -e "/^OUTPUT_FORMAT/d" > $(tmpf)
	@ld $(LDFLAGS) -T$(tmpf) $^ -o $(tmpbin)
	size -A -x $(tmpbin) 
	@rm $(tmpf) $(tmpbin)

clean:
	-rm *.o boot_trampoline.bin
