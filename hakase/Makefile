include build_rule.mk
ifeq ($(HOST),)

.DEFAULT_GOAL:=all

.PHONY: all test clean format prepare_test execute_test start_qemu deploy_to_qemu prepare_script run_script kill_qemu

all:
	cd FriendLoader; $(MAKE) all
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir) all &&) :

test:
	$(MAKE) prepare_test
	$(foreach dir, $(MODULES), $(MAKE) -C $(dir)/test pre-deploy &&) :
	$(MAKE) common_test
	$(MAKE) execute_test

common_test:
	$(MAKE) -C ../common/tests test

clean:
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir) clean;)
	@$(foreach dir, $(MODULES), $(MAKE) -C $(dir)/test clean;)
	make -C FriendLoader clean

format:
	@echo "Formatting with clang-format. Please wait..."
	@git ls-files .. \
	  | grep FriendLoader \
	  | grep -E ".*\.cc$$|.*\.h$$" \
		| xargs -n 1 clang-format -i
	@echo "Done."

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
	/root/qemu > /dev/null 2>&1 &

deploy_to_qemu:
	rsync -r $(DEPLOY_DIR)/* rsync://localhost:2873/share

prepare_script:
	touch $(RUN_SCRIPT)
	echo "#!/bin/bash -e" > $(RUN_SCRIPT)
	chmod +x $(RUN_SCRIPT)

run_script:
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo  Executing baremetal codes on QEMU.
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo /root/ssh $(QEMU_DIR)/script.sh
	@bash -c '/root/ssh $(QEMU_DIR)/script.sh || { echo "" | netcat localhost 2222 > /dev/null 2>&1 || echo -e "\e[31m**** The CPU has been reset due to a critical error. ****\e[m"; exit 1; }'

endif
