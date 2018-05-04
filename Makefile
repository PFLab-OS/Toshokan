CXXFLAGS = -g -O0 -Wall --std=c++14

init: init.cc
	g++ $(CXXFLAGS) $^ -o $@

callback: callback.cc
	g++ $(CXXFLAGS) $^ -o $@

print: print.cc
	g++ $(CXXFLAGS) $^ -o $@

exec_bin: exec_bin.cc
	g++ $(CXXFLAGS) $^ -o $@

mem_rw: mem_rw.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	cd ../FriendLoader; ./run.sh load;
	make init; ./test.sh ./init
	make callback; ./test.sh ./callback
	make print; ./test.sh ./print
	make mem_rw; ./test.sh ./mem_rw
	#make exec_bin; ./test.sh ./exec_bin
	cd ../FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"
