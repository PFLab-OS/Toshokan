CFLAGS = -Wall -Wextra -Wconversion -pie -fPIE -static

main.elf: kernel.ld startup.o main.o uart.o time.o
	ld -T$^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) -c $< -o $@

clean:
	rm -f main.elf *.o
