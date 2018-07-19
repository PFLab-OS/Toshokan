SRC:=main.cc align.cc channel_accessor.cc mock/channel.cc mock/debug.cc #result.cc memrw.cc 

.PHONY: test clean

test: $(OBJ)
	g++ $(SRC) --std=c++14 --coverage -iquote mock -I mock -iquote /share/ -iquote /share/hakase/ -I/cpputest/include -L/cpputest/lib -lCppUTest -lCppUTestExt
	./a.out -c

clean:
	rm -f ./a.out *.gcov *.gcda *.gcno
