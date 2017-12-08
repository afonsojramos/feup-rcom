CC=gcc
CFLAGS=-Wall -Wextra -g
CFLAGSD=$(CFLAGS) -DDEBUG #debug flags
app: download_task

download_task: download urlParser clientTCP
	$(CC) download.o urlParser.o clientTCP.o -o download $(CFLAGS)

download: download.c 
	$(CC) -c download.c $(CFLAGS)


urlParser: urlParser.h urlParser.c 
	$(CC) -c urlParser.c $(CFLAGS)


clientTCP: clientTCP.c 
	$(CC) -c clientTCP.c $(CFLAGS)


clean:
	rm download urlParser.o

#make instructions
defaultUrl=ftp://anonymous:password@ftp.fe.up.pt/welcome.msg
run: download_task #run the file
	clear
	./download $(defaultUrl)

runD: download_taskD #run the file in Debug mode
	clear
	./download $(defaultUrl)
