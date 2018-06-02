CXXFLAGS = -g -O0 -Wall --std=c++14
TESTS = data_size invalid reading_signature rw

default: test

%.bin: %.cc ../test.cc
	g++ -iquote .. -iquote ../include $(CXXFLAGS) $^ -o $@

test:
	@$(foreach test, $(TESTS), make $(test).bin; ../test_hakase.sh 0 $(shell pwd)/$(test).bin; )

clean:
	-rm *.bin
