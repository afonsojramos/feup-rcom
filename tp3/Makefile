CC=gcc
CFLAGS=-Wall -g -std=c11 -DDEBUG


app: applet_task

all: receiver_task sender_task

receiver_task: receiver.c utils
	$(CC) receiver.c -o receiver $(CFLAGS)

sender_task: sender.c utils
	$(CC) sender.c -o sender $(CFLAGS)

applet_task: applet.c utils
	$(CC) applet.c -o applet $(CFLAGS)


utils: utils.c
	$(CC) utils.c -c -o utils.o -Wall

clean:
	rm receiver
	rm sender
	rm applet

runR: receiver_task
	clear
	./receiver /dev/ttyS0

runS: sender_task
	clear
	./sender /dev/ttyS0

runP: applet_task
	clear
	./applet maracas.gif #guiao.pdf
