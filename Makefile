CXXFLAGS = -g -O0 -Wall --std=c++14

init: init.cc
	g++ $(CXXFLAGS) init.cc

callback: callback.cc
	g++ $(CXXFLAGS) callback.cc

print: print.cc
	g++ $(CXXFLAGS) print.cc

stop:
	-sudo rmmod friend_loader.ko

reset:
	make stop
	cd ../FriendLoader; ./run.sh

test:
	make reset
	make init
	sudo ./a.out
	make reset
	make callback
	sudo ./a.out
	make reset
	make print
	sudo ./a.out
	make stop
	@echo "All tests have successfully finished!"
