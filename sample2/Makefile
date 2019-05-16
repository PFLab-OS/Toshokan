DEFAULT: hakase.bin

friend.bin: friend_boot.S friend.cc
	bin/friend-g++ -g -O0 -Wall --std=c++14 -nostdinc -nostdlib -I/usr/local/include -L/usr/local/lib64  -fno-pie -no-pie -T /usr/local/etc/friend.ld -o $@ $^ -lfriend

friend_bin.o: friend.bin
	bin/objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $^ $@

friend.shsym: friend.bin
	bin/objcopy -j .shsym $^ $@

hakase.bin: hakase.cc friend_bin.o friend.shsym
	bin/hakase-g++ -g -O0 -Wall --std=c++14 -static -fno-pie -no-pie -Wl,-R,friend.shsym -o $@ /usr/local/etc/hakase.ld friend_bin.o hakase.cc -lhakase
