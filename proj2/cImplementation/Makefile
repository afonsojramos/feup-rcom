CC=g++
CFLAGS=-Wall -g -std=c++11
CFLAGSD=$(CFLAGS) -DDEBUG #debug flags
app: download_task

download_task: download.c urlParser
	$(CC) -o download download.c urlParser.o $(CFLAGS)

urlParser: urlParser.h urlParser.c
	$(CC) -c urlParser.c $(CFLAGS)


clean:
	rm download

#make instructions
defaultUrl=ftp://user:password@fe.up.pt/base/lol/file.json
run: download_task #run the file
	clear
	./download $(defaultUrl)

runD: download_taskD #run the file in Debug mode
	clear
	./download $(defaultUrl)
