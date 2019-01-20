{% set target_fullpath = target | regex_replace('^(.*)$',dir+'/\\1') | normpath -%}
{% set depends_fullpath = depends | map('regex_replace','^(.*)$',dir+'/\\1') | map('normpath') | list -%}
-include {{ depends_fullpath | map('regex_replace','.cc','.d') | select('string') | join(' ') }}

{% set depends_var = "DEPENDS_" + target_fullpath | regex_replace('/', '_') | regex_replace('\.', '_') | upper() -%}
{{ depends_var }}:={{ depends_fullpath | join(' ') }}
{{ target_fullpath }}:$({{ depends_var }})
	@echo "CC: (docker)/{{ dir }} : {{ target }} <=$({{ depends_var }})"
	$(DOCKER_CMD) $(BUILD_CONTAINER) g++ {{ cflags }} -o {{ target_fullpath }} {{ depends_fullpath | join(' ') }}
