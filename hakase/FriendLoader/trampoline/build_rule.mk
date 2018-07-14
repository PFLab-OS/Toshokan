BOOTOBJS := bootentry.o main.o

FLAGS := -Os -nostdinc -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -iquote $(CURDIR)/../.. -D__FRIEND__
CXXFLAGS := $(FLAGS) --std=c++14
CFLAGS := $(FLAGS)
ASFLAGS := -Os
LDFLAGS := -Os -nostdlib
MAKE := $(MAKE) -f build_rule.mk

default: bin.o

bin.o: boot_trampoline.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

boot_trampoline.bin: $(BOOTOBJS)
	@if [ $(shell $(MAKE) check_bin_size | grep Total | sed -e 's/Total//' | xargs -I{} printf "%d" {} ) -gt $(shell printf "%d" 0x1000) ]; then \
	 echo "[Fatal Error] The trampoline size is too big!"; exit 1 ;\
	fi	
	ld $(LDFLAGS) -Tboot_trampoline.ld $^ -o $@

check_bin_size: $(BOOTOBJS)
	-$(eval tmpf := $(shell mktemp))
	-$(eval tmpbin := $(shell mktemp))
	@cat boot_trampoline.ld | sed -e "/^OUTPUT_FORMAT/d" > $(tmpf)
	@ld $(LDFLAGS) -T$(tmpf) $^ -o $(tmpbin)
	size -A -x $(tmpbin) 
	@rm $(tmpf) $(tmpbin)

clean:
	rm -f *.o boot_trampoline.bin
