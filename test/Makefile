TESTS = callback print
INCLUDE_DIR = $(PWD)/../include
CXXFLAGS = -g -O0 -Wall --std=c++14 -iquote $(INCLUDE_DIR)
export INCLUDE_DIR
export CXXFLAGS

default: test

init.bin: init.cc
	g++ $(CXXFLAGS) $^ -o $@

%.bin: %.cc test.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	make -C result test
	cd ../FriendLoader; ./run.sh load;
	make init.bin; ./test_hakase.sh 0 ./init.bin
	@$(foreach test, $(TESTS), make $(test).bin; ./test_hakase.sh 0 ./$(test).bin; )
	make -C memrw test
	make -C loader test
	cd ../FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"

clean:
	rm -f *.bin
	make -C memrw clean
	make -C result clean
	make -C loader clean
