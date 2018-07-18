test:
	g++ main.cc result.cc --std=c++14 --coverage -iquote /share/hakase/ -I/cpputest/include -L/cpputest/lib -lCppUTest -lCppUTestExt
	./a.out

clean:
	rm -f ./a.out *.gcov *.gcda *.gcno
