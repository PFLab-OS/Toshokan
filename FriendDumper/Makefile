obj-m := friend_dumper.o
friend_dumper-objs := main.o dumper_dev.o

ifndef KERN_SRC
$(error define KERN_SRC)
endif

.PHONY: all clean

all:
	make -C $(KERN_SRC) M=$(PWD) modules

clean:
	make -C $(KERN_SRC) M=$(PWD) clean
