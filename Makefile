CXXFLAGS = -g -O0 -Wall --std=c++14

init: init.cc
	g++ $(CXXFLAGS) $^ -o $@

callback: callback.cc
	g++ $(CXXFLAGS) $^ -o $@

print: print.cc
	g++ $(CXXFLAGS) $^ -o $@

exec_bin: exec_bin.cc
	g++ $(CXXFLAGS) $^ -o $@

test:
	cd ../FriendLoader; ./run.sh load; ./run.sh run
	make init; sudo ./init
	cd ../FriendLoader; ./run.sh restart
	make callback; sudo ./callback
	cd ../FriendLoader; ./run.sh restart
	make print; sudo ./print
	cd ../FriendLoader; ./run.sh restart
	#make exec_bin; sudo ./exec_bin
	cd ../FriendLoader; ./run.sh unload
	@echo "All tests have successfully finished!"
