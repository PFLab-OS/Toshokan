.PHONY: build_rules.mk

default:

build_rules.mk:
	-rm -rf build_rules.mk
	docker rm -f toshokan_rule_generator || :
	docker run -it -d -w /workdir --name toshokan_rule_generator livadk/jinja2:ae8602a4bdc5a04cecb552d01e717233eae46fa3 sh
	rsync --blocking-io -q -e 'docker exec -i' -rltDv . toshokan_rule_generator:/workdir
	docker exec -i toshokan_rule_generator python3 /workdir/rule_generator/gen_build_rules.py
	rsync --blocking-io -q -e 'docker exec -i' -rltDv toshokan_rule_generator:/workdir/ .
	docker rm -f toshokan_rule_generator
	chmod -w build_rules.mk

Makefile: ;

%: build_rules.mk
	$(MAKE) -f build_rules.mk $@
