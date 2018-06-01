CXXFLAGS = -g -O0 -Wall --std=c++14
TESTS = callback print 

default: test

init.bin: init.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

%.bin: %.cc test.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

test:
	make -C result test
	cd ./FriendLoader; ./run.sh load;
	make init; ./test_hakase.sh 0 ./init
	@$(foreach test, $(TESTS), make $(test).bin; ./test_hakase.sh 0 ./$(test).bin; )
	make -C memrw test
	make -C loader test
	cd ./FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"

clean:
	-rm init *.bin
	make -C memrw clean
	make -C result clean
	make -C loader clean
