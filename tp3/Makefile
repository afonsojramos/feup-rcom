CC=gcc
CFLAGS=-Wall -DDEBUG -std=c11

all: receiver_task sender_task

receiver_task: receiver.c utils
	$(CC) receiver.c -o receiver $(CFLAGS)

sender_task: sender.c utils
	$(CC) sender.c -o sender $(CFLAGS)


utils: utils.c
	$(CC) utils.c -c -o utils.o -Wall

clean:
	rm receiver
	rm sender

runR: receiver_task
	./receiver /dev/ttyS0

runS: sender_task
	./sender /dev/ttyS0
