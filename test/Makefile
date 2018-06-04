BUILD_RULE_FILE := build_rule.mk

define run_remote
	vagrant ssh -c "$(1)"
endef

define make_wrapper
	$(if $(shell if [ -e /etc/hakase_installed ]; then echo "guest"; fi), \
	  # guest environment
    cd /vagrant; $(MAKE) -f $(BUILD_RULE_FILE) $(1), \
	  # host environment
	  @echo Running \"make $(1)\" on the remote build environment.
	  @vagrant status | grep running > /dev/null 2>&1 || vagrant reload
	  $(call run_remote, pwd=$(shell pwd); cd /vagrant\$${pwd#$$(vagrant status | grep host_dir: | cut -f2)}; env MAKEFLAGS="$(MAKEFLAGS)" make -f $(BUILD_RULE_FILE) $(1))
	)
endef

.PHONY: test clean

default: test

test:
	$(call make_wrapper,test)


