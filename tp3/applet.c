#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <assert.h> // should be removed after all asserts have been tested.

#include "app_r_utils.c"
#include "app_s_utils.c"
#include "defines.h"

typedef struct{
	int fd_port;
	char port[20];
	int fd_file_send;
	int fd_file_create;
} config;

char sendFile(int fd, char * filename);
char readFile(int fd);

int main(int argc, char **argv){
	struct termios oldtio,newtio;
  config cfg = { .fd_port = 0, .fd_file_send = 0, .fd_file_create = 0};

	/*fflush(stdout);
	int total = 200000;
	for(int i = 0; i < total; i++){
		displayProgress(i, total);
	}*/

	printf("Sender(1) or Receiver(0)?\n");
	int stat;
	scanf("%d", &stat);
	printf("%d\n", stat);

	if(stat != 0 && stat != 1){
		printf("Invalid instruction (0 for sender and 1 for receiver)\n");
		exit(1);
	}

	if(stat == 1 && argc != 2){
		printf("No file specified in the command arguments\nUsage: ./applet <fileToSend>\n");
		exit(2);
	}

	//Choose port
	printf("\n Which PORT do you want to use? \n(0)--(/dev/ttyS0)  (1)--(/dev/ttyS1).\n");
	char option;
	scanf(" %c", &option);
	switch(option){
	  case '0': strcpy(cfg.port,"/dev/ttyS0"); break;
	  case '1': strcpy(cfg.port,"/dev/ttyS1"); break;
		default: printf("Invalid port chosen\n"); exit(3);
	}

	//Open port
	cfg.fd_port = open(cfg.port, O_RDWR | O_NOCTTY );
	if (cfg.fd_port <0){
			perror(cfg.port);
	    return -1;
	}

	if (tcgetattr(cfg.fd_port,&oldtio) == -1){
	    printf("\nError reading port settings.\n");
	    return -1;
	}


    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN] = 1;   /* blocking read until 1 char received */

    tcflush(cfg.fd_port, TCIOFLUSH);

    if (tcsetattr(cfg.fd_port, TCSANOW, &newtio) == -1) {
      perror("tcsetattr failed");
	    return -1;
    }

		int result;

		if (stat == 1){  // Big fork here
	    printf("Ready to send\n");
			result = sendFile(cfg.fd_port, argv[1]);
		} else{
			printf("Ready to receive\n");
			result = readFile(cfg.fd_port);
		}

		if(result){
			printf("File transmition was successful!\n");
			return 0;
		}else{
			perror("could not transmit file");
			return -1;
		}
}

char sendFile(int fd, char * filename){
	printf("[Sender] Running.\n[S] Sending %s.\n", filename);

	FILE* f=fopen(filename, "r");

	//get file size
	unsigned int fSize;
	fseek(f, 0, SEEK_END);
	fSize=ftell(f);
	rewind(f);
	printf("[S] File to send is %d bytes long.\n", fSize);

	//open connection
	llopenS(fd);

	sendControl(fd, TRUE, filename, fSize); // true for start

	// we will be using 512bytes packets.
	int STOP=0;
	while(!STOP){
		char bytes[FRAME_SIZE];
		unsigned int bytesRead=fread(bytes, 1, FRAME_SIZE, f);
		DEBUG_PRINT("[S] Sending %d bytes.\n", bytesRead);
		sendData(fd, bytes, bytesRead);
		//exit(-1);
		if(bytesRead<FRAME_SIZE){
			// we got to the eof.
			STOP=1;
		}
	}

	// the whole file has been sent. Sending CONTROL END PACKET

	sendControl(fd, FALSE, filename, fSize); // false for END




	return TRUE;
}

char readFile(int fd){
	printf("[Receiver] Running.\n");

	int llop=llopenR(fd);

	if(llop!=0){
		fprintf(stderr, "[R] Could not establish connection.");
		exit(-1);
	}

	// should get a control packet with the filename and its size.
	rfile received;
	char control = getPacket(fd, &received);

	if(control!=2){ // 2 is START
		fprintf(stderr, "[R] Didn't get a START CONTROL PACKET when expected.");
		exit(-2);
	}

	// let's prepare the way for that file we'll receive.

	printf("[R] Receiving file %s. It is %d bytes long.\n", received.name, received.size);
	FILE *f;
	f = fopen(received.name, "w");

	//we can get rid of the name now.
	free(received.name);

	// now, we should get the file.

	char gpRet=0;
	while(1){
		gpRet=getPacket(fd, &received);
		if(gpRet==3)
			break;
		DEBUG_PRINT("writting %d bytes to file.\n", received.cSize);
		printB(received.content, received.cSize);
		fwrite(received.content, 1, received.cSize, f);
		free(received.content);
		fflush(f);
	}

	// we got here, so we most certainly got a CONTROL END PACKET
	// we could check if the file size is what it is supposed to be.

	free(received.name);
	DEBUG_PRINT("ftell(f): %lu vs received.size: %d\n", ftell(f), received.size);
	assert(ftell(f) == received.size);
	fclose(f);

	llcloseR(); // this might not be necessary. We'll see


	return TRUE;
}
