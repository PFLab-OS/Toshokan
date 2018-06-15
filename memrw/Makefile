include ../build_rule.mk

ifeq ($(HOST),)

MODULE_NAME = lib$(notdir $(CURDIR)).a
OBJS := 
DEPS := $(OBJS:%.o=%.d)
.DEFAULT_GOAL:=all

-include $(DEPS)

.PHONY: all

all:

clean:
	rm -f $(MODULE_DIR)$(MODULE_NAME) $(OBJS) $(DEPS)

endif
