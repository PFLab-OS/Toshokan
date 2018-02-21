obj-m := friend_loader.o
friend_loader-objs := main.o cpu_hotplug.o deploy.o deploy_dev.o trampoline.o

ifndef KERN_SRC
$(error define KERN_SRC)
endif

.PHONY: all clean

all:
	make -C $(KERN_SRC) M=$(PWD) modules

clean:
	make -C $(KERN_SRC) M=$(PWD) clean
