obj-m := friend_loader.o
friend_loader-objs := main.o cpu_hotplug.o deploy.o deploy_interface.o call_interface.o trampoline_loader.o trampoline/bin.o
KERN_SRC ?= /lib/modules/$(shell uname -r)/build

.PHONY: all clean trampoline/bin.o

all: trampoline/bin.o
	$(MAKE) -C $(KERN_SRC) M=$(PWD) modules

trampoline/bin.o:
	$(MAKE) -C trampoline

clean:
	$(MAKE) -C $(KERN_SRC) M=$(PWD) clean
	$(MAKE) -C trampoline clean
