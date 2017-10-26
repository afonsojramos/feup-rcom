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

		if (stat == 1){
	    printf("Ready to send\n");
			result = sendFile(cfg.fd_port, argv[1]);
		} else{
			printf("Ready to receive\n");
			result = readFile(cfg.fd_port);
		}

		if(result){
			perror("could not transmit file");
			return result;
		}

}

char sendFile(int fd, char * filename){
	printf("Sending file %s.\n", filename);
	return TRUE;
}

char readFile(int fd){
	printf("Receiving file.\n");
	return TRUE;
}
