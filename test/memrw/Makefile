TESTS = data_size invalid reading_signature rw

default: test

%.bin: %.cc ../test.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	@$(foreach test, $(TESTS), make $(test).bin; ../test_hakase.sh 0 $(shell pwd)/$(test).bin; )

clean:
	rm -f *.bin
