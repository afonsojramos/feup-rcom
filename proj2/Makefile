CC=g++
CFLAGS=-Wall -g
CFLAGSD=$(CFLAGS) -DDEBUG #debug flags
app: download_task

download_task: download.cpp
	$(CC) download.cpp -o download $(CFLAGS)

download_taskD: download.cpp
	$(CC) download.cpp -o download $(CFLAGSD)


clean:
	rm download
#make instructions
defaultUrl=pinguim.gif
run: download_task #run the file
	clear
	./download $(defaultUrl)

runD: download_task #run the file in Debug mode
	clear
	./download $(defaultUrl)
