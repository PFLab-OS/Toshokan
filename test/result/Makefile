NORMAL_TESTS = unwrap_when_noerror ignore_when_error
ABORT_TESTS = unwrap_when_error notchecked_when_noerror notchecked_when_error ignore_when_noerror 

default: test

%.bin: %.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	@$(foreach test, $(NORMAL_TESTS), make $(test).bin; ../test_library.sh 0 $(shell pwd)/$(test).bin; )
	@$(foreach test, $(ABORT_TESTS), make $(test).bin; ../test_library.sh 1 $(shell pwd)/$(test).bin; )

clean:
	rm -f $(foreach test, $(NORMAL_TESTS) $(ABORT_TESTS), $(test).bin)
