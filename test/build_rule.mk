TESTS = callback print
INCLUDE_DIR = $(CURDIR)/..
CXXFLAGS = -g -O0 -Wall --std=c++14 $(foreach dir, $(INCLUDE_DIR), -iquote $(dir))
MAKE := $(MAKE) -f build_rule.mk
export INCLUDE_DIR
export CXXFLAGS

default: test

init.bin: init.cc
	g++ $(CXXFLAGS) $^ -o $@

print.bin: print.cc test.cc ../print/hakase.cc
	g++ $(CXXFLAGS) $^ -o $@

%.bin: %.cc test.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	$(MAKE) -C result test
	cd ../FriendLoader; make all; ./run.sh load;
	$(MAKE) init.bin; ./test_hakase.sh 0 ./init.bin
	@$(foreach test, $(TESTS), $(MAKE) $(test).bin && ./test_hakase.sh 0 ./$(test).bin && ) :
	$(MAKE) -C memrw test
	$(MAKE) -C simple_loader test
	cd ../FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"

clean:
	rm -f *.bin
	cd ../FriendLoader; make clean
	$(MAKE) -C memrw clean
	$(MAKE) -C result clean
	$(MAKE) -C simple_loader clean