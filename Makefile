obj-m := friend_loader.o
friend_loader-objs := main.o cpu_hotplug.o deploy.o deploy_dev.o trampoline_loader.o trampoline/bin.o

ifndef KERN_SRC
$(error define KERN_SRC)
endif

.PHONY: all clean trampoline/bin.o

all: trampoline/bin.o
	$(MAKE) -C $(KERN_SRC) M=$(PWD) modules

trampoline/bin.o:
	$(MAKE) -C trampoline

clean:
	$(MAKE) -C $(KERN_SRC) M=$(PWD) clean
	$(MAKE) -C trampoline clean
