DOCKER_IMAGE_TAG=2fac00a686bf6124772f611096cdc01060c267e0
ROOT_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
RELATIVE_DIR:=$(shell bash -c "root_dir=$(abspath $(ROOT_DIR)../); pwd=$(CURDIR); echo \$${pwd\#\$${root_dir}};")
DEPLOY_DIR:=/deploy/
RUN_SCRIPT:=$(DEPLOY_DIR)/script.sh
QEMU_DIR:=/home/ubuntu/share
HOST=$(shell if [ ! -e /lib/modules/4.14.34hakase/build ]; then echo "host"; fi)

ifneq ($(HOST),)
# host environment

define make_wrapper
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@echo  Running \"make$1\" on the docker environment.
	@echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	@docker rm hakase_devenv -f > /dev/null 2>&1 || :
	docker run -d --mount type=tmpfs,destination=/ram,tmpfs-size=400M --cpu-period=50000 --cpu-quota=45000 -it -v $(abspath $(ROOT_DIR)../):/share --name hakase_devenv livadk/hakase-qemu:$(DOCKER_IMAGE_TAG)
	@echo ""
	docker exec -t hakase_devenv sh -c "cp /root/*.diff.qcow2 /ram"
	docker exec -t -w /share$(RELATIVE_DIR) hakase_devenv make$1
	@echo ""
	docker rm -f hakase_devenv
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
