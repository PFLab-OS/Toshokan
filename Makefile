CXXFLAGS = -g -O0 -Wall --std=c++14

default: test

init: init.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

callback: callback.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

print: print.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

memrw: memrw.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

test:
	make -C result test
	cd ../FriendLoader; ./run.sh load;
	make init; ./test_hakase.sh ./init
	make callback; ./test_hakase.sh ./callback
	make print; ./test_hakase.sh ./print
	make memrw; ./test_hakase.sh ./memrw
	make -C loader test
	cd ../FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"

clean:
	-rm init callback print memrw
	make -C result clean
	make -C loader clean
