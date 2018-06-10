include ../build_rule.mk

ifeq ($(HOST),)

MODULE_NAME = lib$(notdir $(CURDIR)).a
TESTS = hakase
.DEFAULT_GOAL:=all

.PHONY: all

all: $(BUILD_DIR)$(MODULE_NAME)

$(BUILD_DIR)$(MODULE_NAME): $(BUILD_DIR) hakase.o
	ar rcs $(BUILD_DIR)$(MODULE_NAME) hakase.o

clean:
	rm -f $(BUILD_DIR)$(MODULE_NAME) *.o

endif
