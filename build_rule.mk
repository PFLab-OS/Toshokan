MAKEFLAGS+=--no-print-directory

HOST=$(shell if [ ! -e /etc/hakase_installed ]; then echo "host"; fi)

ifneq ($(HOST),)
# host environment
VAGRANT_ROOT_DIR=$(shell vagrant status | grep host_dir: | cut -f2)

define run_remote
	@vagrant ssh -c "$(1)" | grep -v "host_dir:\t$(VAGRANT_ROOT_DIR)"
endef

define make_wrapper
	@echo Running \"make$1\" on the remote build environment.
	@vagrant status | grep running > /dev/null 2>&1 || vagrant reload
	$(call run_remote, root_dir=$(VAGRANT_ROOT_DIR); pwd=$(CURDIR); cd /vagrant\$${pwd#\$${root_dir}}; env MAKEFLAGS='$(MAKEFLAGS)' make$1)
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
ROOT_DIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
TEST_DIR=$(ROOT_DIR)tests/
BUILD_DIR = $(ROOT_DIR)build/

CXXFLAGS = -g -O0 -Wall --std=c++14 -iquote $(ROOT_DIR)

load:
ifeq ($(RECURSIVE),)
	@echo "info: starting FriendLoader"
	@cd $(ROOT_DIR)FriendLoader; make all; ./run.sh load
endif

unload:
ifeq ($(RECURSIVE),)
	@echo "info: Stopping FriendLoader"
	@cd $(ROOT_DIR)FriendLoader; ./run.sh unload
endif

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

endif
