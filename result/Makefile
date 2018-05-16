CXXFLAGS = -g -O0 -Wall --std=c++14
NORMAL_TESTS = noerror error test_ignore_when_error
ABORT_TESTS = unwrap_when_error notchecked_when_noerror notchecked_when_error test_ignore_when_noerror 

default: test

test:
	@$(foreach test, $(NORMAL_TESTS), rm -f $(test); g++ -iquote .. $(CXXFLAGS) $(test).cc -o $(test); ../test_library.sh ./$(test) 0; )
	@$(foreach test, $(ABORT_TESTS), rm -f $(test); g++ -iquote .. $(CXXFLAGS) $(test).cc -o $(test); ../test_library.sh ./$(test) 1; )

clean:
	-rm $(NORMAL_TESTS) $(ABORT_TESTS)
