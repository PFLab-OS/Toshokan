include ../build_rule.mk

ifeq ($(HOST),)

MODULE_NAME = lib$(notdir $(CURDIR)).a
OBJS := hakase.o
DEPS := $(OBJS:%.o=%.d)
.DEFAULT_GOAL:=all

-include $(DEPS)

.PHONY: all

all: $(MODULE_DIR)$(MODULE_NAME)

$(MODULE_DIR)$(MODULE_NAME): $(MODULE_DIR) $(OBJS)
	ar rcs $(MODULE_DIR)$(MODULE_NAME) $(OBJS)

clean:
	rm -f $(MODULE_DIR)$(MODULE_NAME) $(OBJS) $(DEPS)

endif
