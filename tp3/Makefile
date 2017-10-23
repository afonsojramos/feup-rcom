CC=gcc
CFLAGS=-Wall -DDEBUG

receiver_task: receiver.c
	$(CC) receiver.c -o receiver $(CFLAGS)

clean:
	rm receiver

run: receiver_task
	./receiver /dev/ttyS0
