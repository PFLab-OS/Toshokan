.DEFAULT_GOAL:hakase/interrupt/interrupt.o
.PHONY: 

CC:=
CXX:=

{% for rule in rules %}
{{ rule }}
{% endfor %}

clean:
	rm -f Makefile {% for file in clean_targets %} {{ file }}{% endfor %}

