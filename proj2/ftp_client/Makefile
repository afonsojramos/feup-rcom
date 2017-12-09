CC=gcc
CFLAGS=-Wall -Wextra -g
CFLAGSD=$(CFLAGS) -DDEBUG #debug flags

.PHONY:run

download: download.o urlParser.o clientTCP.o
	$(CC) download.o urlParser.o clientTCP.o -o download $(CFLAGS)

download.o: download.c download.h
	$(CC) -c download.c $(CFLAGS)

urlParser.o: urlParser.h urlParser.c 
	$(CC) -c urlParser.c $(CFLAGS)


clientTCP.o: clientTCP.c clientTCP.h
	$(CC) -c clientTCP.c $(CFLAGS)


clean:
	rm download urlParser.o clientTCP.o download.o

#make instructions
defaultUrl=ftp://anonymous:password@ftp.fe.up.pt/welcome.msg
run: download #run the file
	clear
	./download $(defaultUrl)
