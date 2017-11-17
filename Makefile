include common.mk

define make_wrapper
	$(if $(shell if [ -e /etc/vagrant_setup ]; then echo "guest"; fi), \
	  # VirtualBox guest VM
	  cd /vagrant/; \
	  $(MAKE) -f $(RULE_FILE) $(1), \

	  # Host
	  $(if $(shell ssh -F .ssh_config default "exit"; \
	  		if [ $$? != 0 ]; then echo "no-guest"; fi), \
	  	vagrant halt
	  	vagrant up)
	  ssh -F .ssh_config default "cd /vagrant/; \
		  env MAKEFLAGS=$(MAKEFLAGS) make -f $(RULE_FILE) $(1)"
	)
endef

.PHONY: qemu

qemu:
	$(call make_wrapper,qemu)
