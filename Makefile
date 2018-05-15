CXXFLAGS = -g -O0 -Wall --std=c++14

default: test

init: init.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

callback: callback.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

print: print.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

loader/exec: loader/exec.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

mem_rw: mem_rw.cc
	g++ -iquote . $(CXXFLAGS) $^ -o $@

test:
	cd ../FriendLoader; ./run.sh load;
	make init; ./test.sh ./init
	make callback; ./test.sh ./callback
	make print; ./test.sh ./print
	make mem_rw; ./test.sh ./mem_rw
	make loader/exec; ./test.sh ./loader/exec
	cd ../FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"
