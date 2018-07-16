ccflags-y += -iquote $(PWD)/..
obj-m := friend_loader.o
friend_loader-objs := main.o cpu_hotplug.o deploy.o call_interface.o debug_mem.o trampoline_loader.o trampoline/bin.o
KERN_SRC ?= /lib/modules/4.14.34hakase/build

.PHONY: all clean FORCE

FORCE:

all: trampoline/bin.o
	$(MAKE) -C $(KERN_SRC) M=$(CURDIR) modules

trampoline/bin.o: FORCE
	$(MAKE) -f build_rule.mk -C trampoline

pre-deploy:
	cp run.sh friend_loader.ko /deploy/

clean:
	$(MAKE) -C $(KERN_SRC) M=$(CURDIR) clean
	$(MAKE) -f build_rule.mk -C trampoline clean
