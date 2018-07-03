include build_rule.mk
ifeq ($(HOST),)

.DEFAULT_GOAL:=all

.PHONY: all test

all:
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir) all &&) :

test:
	$(MAKE) load
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir)/test test &&) :
	$(MAKE) unload
	@echo "All tests have successfully finished!"

clean:
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir) clean;)
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir)/test clean;)
	make -C FriendLoader clean

endif
