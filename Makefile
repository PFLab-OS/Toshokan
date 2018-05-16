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
	cd ../FriendLoader; ./run.sh load;
	make init; ./test.sh ./init
	make callback; ./test.sh ./callback
	make print; ./test.sh ./print
	make memrw; ./test.sh ./memrw
	make -C loader test
	cd ../FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"

clean:
	-rm init callback print memrw
	make -C loader clean
