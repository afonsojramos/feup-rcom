CC=g++
CFLAGS=-Wall -g -std=c++11
CFLAGSD=$(CFLAGS) -DDEBUG #debug flags
app: download_task

download_task: download.cpp
	$(CC) download.cpp -o download $(CFLAGS)

download_taskD: download.cpp
	$(CC) download.cpp -o download $(CFLAGSD)


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
