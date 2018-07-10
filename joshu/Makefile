include ../hakase/build_rule.mk

ifeq ($(HOST),)

MODULE_NAME = $(notdir $(CURDIR)).bin
OBJS := joshu.o int.o int_asm.o
DEPS := $(OBJS:%.o=%.d)
.DEFAULT_GOAL:=all

-include $(DEPS)

.PHONY: all

all: $(MODULE_NAME)

$(MODULE_NAME): joshu.cc int.cc int_asm.S 
	g++ -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote $(ROOT_DIR) -T joshu.ld $^ -o $@

clean:
	rm -f $(MODULE_NAME) 

endif
