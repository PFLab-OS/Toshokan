DOCKER_IMAGE_TAG=c07f6982495230c5377f0decb114a7fac7908c09
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
CONTAINER_NAME:=hakase_devenv

define make_wrapper
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo  Running \"make$1\" on the docker environment.
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@docker rm $(CONTAINER_NAME) -f > /dev/null 2>&1 || :
	docker run -d $(if $(CI),,-v $(HOST_DIR):$(SHARE_DIR)) -it --name $(CONTAINER_NAME) livadk/hakase-qemu:$(DOCKER_IMAGE_TAG)
	$(if $(CI),docker cp $(HOST_DIR) $(CONTAINER_NAME):$(SHARE_DIR))
	@echo ""
	docker exec -t $(CONTAINER_NAME) sh -c "cd /share$(RELATIVE_DIR) && make$1"
	@echo ""
	docker rm -f $(CONTAINER_NAME)
endef

.DEFAULT_GOAL:=default
default:
	$(call make_wrapper,)

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

CXXFLAGS = -g -O0 -MMD -MP -Wall --std=c++14 -iquote $(ROOT_DIR)

$(MODULE_DIR):
	mkdir -p $(MODULE_DIR)

endif
