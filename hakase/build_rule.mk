DOCKER_IMAGE_TAG=63e91ec8a3169b4d2a09b4be0c0d4ba1528152be
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
CONTAINER_NAME:=toshokan

ifeq ($(OS),Windows_NT)
CRLF_CHECK:=! find $(HOST_DIR)/hakase -not -type d -exec file "{}" ";" | grep CRLF || sh -c 'echo "****CRLF detected from the files!****\nplease follow these steps!(Warning: It will discard your changes)\n$$ git config core.autocrlf false\n$$ git reset --hard HEAD"; exit 1'
endif

define make_wrapper
	@$(CRLF_CHECK)
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo  Running \"make$1\" on the docker environment.
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@docker rm $(CONTAINER_NAME) -f > /dev/null 2>&1 || :
	docker run -d $(if $(CI),,-v $(HOST_DIR):$(SHARE_DIR)) -it --name $(CONTAINER_NAME) livadk/hakase-qemu:$(DOCKER_IMAGE_TAG)
	$(if $(CI),docker cp $(HOST_DIR) $(CONTAINER_NAME):$(SHARE_DIR))
	@echo ""
	@echo 'docker exec $(CONTAINER_NAME) sh -c "cd /share$(RELATIVE_DIR) && make$1"'
	@bash -c "trap \"docker exec $(CONTAINER_NAME) sh -c 'pkill qemu-system-x86 || :'\" INT ERR; docker exec $(CONTAINER_NAME) sh -c \"cd /share$(RELATIVE_DIR) && make$1\""
	@echo ""
	docker rm -f $(CONTAINER_NAME)
endef

.DEFAULT_GOAL:=default
default:
	$(call make_wrapper,)

attach_docker:
	docker exec -it $(CONTAINER_NAME) /bin/bash

run_docker:
	docker run --rm -v $(HOST_DIR):$(SHARE_DIR) -it livadk/hakase-qemu:$(DOCKER_IMAGE_TAG) /bin/bash

%:
	$(call make_wrapper, $@)
else
# guest environment
ifneq ($(PWD),$(CURDIR))
RECURSIVE=true
endif

TEST_DIR=$(ROOT_DIR)tests/
MODULE_DIR = $(ROOT_DIR)build/

MODULES=callback print memrw simple_loader elf_loader interrupt

CXXFLAGS = -g -O0 -MMD -MP -Wall --std=c++14 -iquote $(ROOT_DIR) -iquote $(ROOT_DIR)../ -D __HAKASE__

$(MODULE_DIR):
	mkdir -p $(MODULE_DIR)

endif
