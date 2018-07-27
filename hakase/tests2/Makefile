include ../build_rule.mk

ifeq ($(HOST),)

SRC:=main.cc ivshmem.cc

.PHONY: test clean

test: $(SRC)
	g++ $(SRC) --std=c++14 --coverage -iquote mock -I mock -iquote /share/ -iquote /share/hakase/ -I/cpputest/include -L/cpputest/lib -lCppUTest -lCppUTestExt
	./a.out -c

clean:
	rm -f ./a.out *.gcov *.gcda *.gcno

endif
