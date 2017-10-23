CC=gcc
CFLAGS=-Wall.

receiver_task: receiver.c
	$(CC) receiver.c -o receiver

clean:
	rm receiver

run: receiver_task
	./receiver /dev/ttyS0
