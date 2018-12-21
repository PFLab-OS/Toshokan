ccflags-y += -iquote $(PWD) -I $(PWD) -iquote $(PWD)/../../friend -iquote $(PWD)/../.. -iquote $(PWD)/.. 
obj-m := friend_loader.o
friend_loader-objs := main.o cpu_hotplug.o deploy.o call_interface.o debug_mem.o trampoline_loader.o trampoline/bin.o
KERN_VAR ?= $(shell uname -r)
KERN_SRC ?= /lib/modules/$(KERN_VERSION)/build

.PHONY: all

all:
	$(MAKE) -C $(KERN_SRC) M=$(CURDIR) modules
