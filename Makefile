obj-m := depftom.o
depftom-objs := main.o cpu_hotplug.o deploy_data.o

ifndef KERN_SRC
$(error define KERN_SRC)
endif

.PHONY: all clean

all:
	make -C $(KERN_SRC) M=$(PWD) modules

clean:
	make -C $(KERN_SRC) M=$(PWD) clean
