CC=gcc
CFLAGS=-Wall -g -std=c11 -DDEBUG

app: applet_task

applet_task: applet.c utils
	$(CC) applet.c -o applet $(CFLAGS)

utils: utils.c
	$(CC) utils.c -c -o utils.o -Wall

clean:
	rm receiver
	rm sender
	rm applet

run: applet_task
	clear
	./applet pinguim.gif #guiao.pdf
