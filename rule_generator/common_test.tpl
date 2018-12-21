{% set files_fullpath = files | map('regex_replace','^(.*)$',dir+'/\\1') | map('normpath') | list -%}
{% set depends_var = "DEPENDS_" + dir | regex_replace('/', '_') | upper() -%}
{{ depends_var }}:={{ files_fullpath | join(' ') }}
common_test: $({{ depends_var }})
	$(DOCKER_CMD) $(COMMON_TEST_CONTAINER) g++ {{ files_fullpath | join(' ') }} $(COMMON_TEST_CFLAGS)
	$(DOCKER_CMD) $(COMMON_TEST_CONTAINER) ./a.out -c -v
