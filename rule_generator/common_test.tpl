common_test: {% for file in files %} {{ file }}{% endfor %}
	{{ docker_cmd }} livadk/cpputest:aac118d572d3c41bc9e3bed32c7ae8c19249784c g++ {% for file in files %} /workdir/{{ file }}{% endfor %} --std=c++14 --coverage -iquote /workdir/common/tests/mock -I /workdir/common/tests/mock -iquote /workdir/ -iquote /workdir/hakase/ -I/cpputest/include -L/cpputest/lib -lCppUTest -lCppUTestExt -pthread
	{{ docker_cmd }} livadk/cpputest:aac118d572d3c41bc9e3bed32c7ae8c19249784c ./a.out -c -v
