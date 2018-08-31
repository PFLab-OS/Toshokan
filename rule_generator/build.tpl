-include {% for file in depends %} {{ file | regex_replace('.cc','.d') }}{% endfor %}

{{ target }}:{% for file in depends %} {{ file }}{% endfor %}
	@echo "CC: (docker)/{{ dir }} : {{ target }} <={% for file in depends %} {{ file }}{% endfor %}"
	@{{ docker_cmd }} livadk/hakase-qemu:c59192ba2d8eabab38a67aa22cb323471c1ec2ad g++ -g -O0 -MMD -MP -Wall --std=c++14 -iquote /workdir/hakase -I /workdir/hakase -iquote /workdir/ -D __HAKASE__ -c -o /workdir/{{ target }} {% for file in depends %} /workdir/{{ file }}{% endfor %}

