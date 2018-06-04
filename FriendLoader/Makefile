ccflags-y += -iquote $(PWD)/../include
obj-m := friend_loader.o
friend_loader-objs := main.o cpu_hotplug.o deploy.o deploy_interface.o call_interface.o debug_mem.o trampoline_loader.o trampoline/bin.o
KERN_SRC ?= /lib/modules/$(shell uname -r)/build

.PHONY: all clean trampoline/bin.o

all: trampoline/bin.o
	$(MAKE) -C $(KERN_SRC) M=$(CURDIR) modules

trampoline/bin.o:
	$(MAKE) -f build_rule.mk -C trampoline

clean:
	$(MAKE) -C $(KERN_SRC) M=$(CURDIR) clean
	$(MAKE) -f build_rule.mk -C trampoline clean
