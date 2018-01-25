main.elf: kernel.ld startup.o main.o uart.o
	ld -T$^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(CFLAGS) -c $< -o $@

clean:
	rm -f main.elf *.o
