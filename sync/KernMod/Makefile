obj-m := depftom.o
depftom-objs := main.o cpu_hotplug.o deploy_data.o
KERN = $(shell uname -r)

.PHONY: all clean

all:
	make -C /lib/modules/$(KERN)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KERN)/build M=$(PWD) clean
