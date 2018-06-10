include build_rule.mk
ifeq ($(HOST),)

SUB_DIRS=simple_loader elf_loader
.DEFAULT_GOAL:=all

.PHONY: all test

all:
	@$(foreach dir, $(SUB_DIRS), $(MAKE) -C $(dir) all &&) :

test:
	$(MAKE) load
	@$(foreach dir, $(SUB_DIRS), $(MAKE) -C $(dir)/test test &&) :
	$(MAKE) unload
	@echo "All tests have successfully finished!"

clean:
	@$(foreach dir, $(SUB_DIRS), $(MAKE) -C $(dir) clean;)
	@$(foreach dir, $(SUB_DIRS), $(MAKE) -C $(dir)/test clean;)

endif
