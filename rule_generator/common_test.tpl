{% set depends_var = "DEPENDS_" + dir | regex_replace('/', '_') | upper() -%}
{{ depends_var }}:={% for file in files %} {{ file }}{% endfor %}
common_test: $({{ depends_var }})
	$(DOCKER_CMD) $(COMMON_TEST_CONTAINER) g++ $(addprefix /workdir/,$({{ depends_var }})) $(COMMON_TEST_CFLAGS)
	$(DOCKER_CMD) $(COMMON_TEST_CONTAINER) ./a.out -c -v
