obj-m := hello.o
KERN = $(shell uname -r)

.PHONY: all clean

all:
	make -C /lib/modules/$(KERN)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KERN)/build M=$(PWD) clean
