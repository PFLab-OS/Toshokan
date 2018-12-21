%:
	docker run --rm -v $(CURDIR):/workdir$(CURDIR) -w /workdir$(CURDIR) livadk/jinja2:ae8602a4bdc5a04cecb552d01e717233eae46fa3 python3 /workdir$(CURDIR)/rule_generator/gen_build_rules.py
	$(MAKE) -f build_rules.mk $@
