include build_rule.mk
ifeq ($(HOST),)

SUB_DIRS=simple_loader elf_loader
.DEFAULT_GOAL:=test

.PHONY: test

test:
	$(MAKE) load
	@$(foreach dir, $(SUB_DIRS), $(MAKE) -C $(dir)/test test &&) :
	$(MAKE) unload
	@echo "All tests have successfully finished!"

clean:
	@$(foreach dir, $(SUB_DIRS), $(MAKE) -C $(dir)/test clean;)

endif
