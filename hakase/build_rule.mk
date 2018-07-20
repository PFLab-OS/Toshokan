ROOT_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
RELATIVE_DIR:=$(shell bash -c "root_dir=$(abspath $(ROOT_DIR)../); pwd=$(CURDIR); echo \$${pwd\#\$${root_dir}};")
DEPLOY_DIR:=/deploy
RUN_SCRIPT:=$(DEPLOY_DIR)/script.sh
QEMU_DIR:=/home/ubuntu/share
HOST=$(shell if [ ! -e /lib/modules/4.14.34hakase/build ]; then echo "host"; fi)

ifneq ($(HOST),)
# host environment
HOST_DIR:=$(abspath $(ROOT_DIR)../)
SHARE_DIR:=/share
BUILD_CONTAINER:=livadk/hakase-qemu:63e6659f7929854cb72aac6223b7ce424de34a6f
BUILD_CONTAINER_NAME:=toshokan
FORMAT_CONTAINER:=livadk/clang-format:9f1d281b0a30b98fbb106840d9504e2307d3ad8f
FORMAT_CONTAINER_NAME:=toshokan_format

ifeq ($(OS),Windows_NT)
CRLF_CHECK:=! find $(HOST_DIR)/hakase -not -type d -exec file "{}" ";" | grep CRLF || sh -c 'echo "****CRLF detected from the files!****\nplease follow these steps!(Warning: It will discard your changes)\n$$ git config core.autocrlf false\n$$ git reset --hard HEAD"; exit 1'
endif

define make_wrapper
	@$(CRLF_CHECK)
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo  Running \"make$1\" on the docker environment.
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@docker rm $1 -f > /dev/null 2>&1 || :
	docker run -d $(if $(CI),,-v $(HOST_DIR):$(SHARE_DIR)) -it --name $1 $2
	$(if $(CI),docker cp $(HOST_DIR) $1:$(SHARE_DIR))
	@echo ""
	@echo 'docker exec $1 sh -c "cd /share$(RELATIVE_DIR) && make$3"'
	@bash -c "trap \"docker exec $1 sh -c 'pkill qemu-system-x86 || :'\" INT ERR; docker exec $1 sh -c \"cd /share$(RELATIVE_DIR) && make$4\""
	@echo ""
	docker rm -f $1
endef

define docker_wrapper
	@docker rm $1 -f > /dev/null 2>&1 || :
	docker run -d $(if $(CI),,-v $(HOST_DIR):$(SHARE_DIR)) -it --name $1 $2
	$(if $(CI),docker cp $(HOST_DIR) $1:$(SHARE_DIR))
	docker exec $1 sh -c "cd /share$(RELATIVE_DIR) && $3"
	@echo ""
	docker rm -f $1
endef

.DEFAULT_GOAL:=default
default:
	$(call make_wrapper,$(BUILD_CONTAINER_NAME),$(BUILD_CONTAINER))

attach_docker:
	docker exec -it $(BUILD_CONTAINER_NAME) /bin/bash

run_docker:
	docker run --rm -v $(HOST_DIR):$(SHARE_DIR) -it $(BUILD_CONTAINER) /bin/bash

format:
	@echo "Formatting with clang-format. Please wait..."
	@$(call docker_wrapper,$(FORMAT_CONTAINER_NAME),$(FORMAT_CONTAINER),\
	 git ls-files .. \
	  | grep -e FriendLoader/ \
	         -e ../friend/ \
	  | grep -E '.*\.cc$$|.*\.h$$' \
		| xargs -n 1 clang-format -style='{BasedOnStyle: Google}' -i \
	 $(if $(CI),&& git diff --quiet))
	@echo "Done."

%:
	$(call make_wrapper,$(BUILD_CONTAINER_NAME),$(BUILD_CONTAINER), $@)
else
# guest environment
ifneq ($(PWD),$(CURDIR))
RECURSIVE=true
endif

TEST_DIR=$(ROOT_DIR)tests/
MODULE_DIR = $(ROOT_DIR)build/

MODULES=callback print memrw simple_loader elf_loader interrupt

CXXFLAGS = -g -O0 -MMD -MP -Wall --std=c++14 -iquote $(ROOT_DIR) -I $(ROOT_DIR) -iquote $(ROOT_DIR)../ -D __HAKASE__

$(MODULE_DIR):
	mkdir -p $(MODULE_DIR)

endif
