CC=gcc
CFLAGS=-Wall -g -std=c11
CFLAGSD=$(CFLAGS) -DDEBUG #debug flags
app: applet_task

applet_task: applet.c
	$(CC) applet.c -o applet $(CFLAGS)

applet_taskD: applet.c
	$(CC) applet.c -o applet $(CFLAGSD)


clean:
	rm applet
	rm utils
#make instructions
defaultFile=pinguim.gif
run: applet_task #run the file
	clear
	./applet $(defaultFile)

runD: applet_taskD #run the file in Debug mode
	clear
	./applet $(defaultFile)

runS: applet_task #run Sender in Debug mode
	clear
	echo "1 0" | ./applet $(defaultFile)

runR: applet_task #run the Receiver in Debug mode
	clear
	echo "0 0" | ./applet $(defaultFile)

runSD: applet_taskD #run Sender in Debug mode
	clear
	echo "1 0" | ./applet $(defaultFile)

runRD: applet_taskD #run the Receiver in Debug mode
	clear
	echo "0 0" | ./applet $(defaultFile)
