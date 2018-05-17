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
	make init; ./test_hakase.sh 0 ./init
	make callback; ./test_hakase.sh 0 ./callback
	make print; ./test_hakase.sh 0 ./print
	make memrw; ./test_hakase.sh 0 ./memrw
	make -C loader test
	cd ../FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"

clean:
	-rm init callback print memrw
	make -C result clean
	make -C loader clean
