include ../../hakase/build_rule.mk

ifeq ($(HOST),)

SRC:=main.cc align.cc result.cc channel.cc channel_accessor2.cc mock/channel.cc mock/debug.cc # memrw.cc channel_accessor.cc

.PHONY: test clean

test: $(SRC)
	g++ $(SRC) --std=c++14 --coverage -iquote mock -I mock -iquote /share/ -iquote /share/hakase/ -I/cpputest/include -L/cpputest/lib -lCppUTest -lCppUTestExt -pthread
	./a.out -c -v

clean:
	rm -f ./a.out *.gcov *.gcda *.gcno

endif
