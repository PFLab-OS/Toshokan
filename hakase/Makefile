include build_rule.mk
ifeq ($(HOST),)

TESTS=$(MODULES) channel

.DEFAULT_GOAL:=all

.PHONY: all test clean format prepare_test execute_test start_qemu deploy_to_qemu prepare_script run_script kill_qemu

all:
	cd FriendLoader; $(MAKE) all
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir) all &&) :

test:
	$(MAKE) prepare_test
	$(foreach dir, $(TESTS), $(MAKE) -C $(dir)/test pre-deploy &&) :
	$(MAKE) common_test
	$(MAKE) execute_test

common_test:
	$(MAKE) -C ../common/tests test
	$(MAKE) -C tests2 test

clean:
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir) clean;)
	@$(foreach dir, $(TESTS), $(MAKE) -C $(dir)/test clean;)
	make -C FriendLoader clean

###########################
#  Sub Functions
###########################
prepare_test:
	$(MAKE) all
	$(MAKE) start_qemu
	rm -rf $(DEPLOY_DIR)
	mkdir $(DEPLOY_DIR)
	$(MAKE) prepare_script
	cp $(TEST_DIR)test_hakase.sh $(DEPLOY_DIR)
	cp $(TEST_DIR)test_library.sh $(DEPLOY_DIR)
	cd FriendLoader; $(MAKE) pre-deploy

execute_test:
	$(MAKE) deploy_to_qemu
	$(MAKE) run_script
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo "All tests have successfully finished!"
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"

start_qemu:
	/root/qemu

deploy_to_qemu:
	/root/rsync $(DEPLOY_DIR)/\* share

prepare_script:
	touch $(RUN_SCRIPT)
	echo "#!/bin/bash -e" > $(RUN_SCRIPT)
	chmod +x $(RUN_SCRIPT)

run_script:
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo  Executing baremetal codes on QEMU.
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo /root/ssh $(QEMU_DIR)/script.sh
	@bash -c '/root/ssh $(QEMU_DIR)/script.sh || { echo "" | /root/netcat > /dev/null 2>&1 || echo -e "\e[31m**** The CPU has been reset due to a critical error. ****\e[m"; exit 1; }'

endif
