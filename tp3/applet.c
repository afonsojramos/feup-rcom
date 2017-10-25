#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef struct{
	int fd_port;
	char port[20];
	int fd_file_send;
	int fd_file_create;
} config;

int main(int argc, char **argv){

  config cfg = { .fd_port = 0, .fd_file_send = 0, .fd_file_create = 0};

	printf("Sender(1) or Receiver(0)?\n");
	scanf("%d", &stat);
	printf("%d\n", stat);

	if(stat == 0 || stat == 1){

	}

	//Choose port
	printf("\n Which PORT do you want to use? \n(0)--(/dev/ttyS0)  (1)--(/dev/ttyS1).\n");
	scanf(" %c", &option);
	switch(option){
	  case '0': strcpy(cfg.port,"/dev/ttyS0"); break;
	  case '1': strcpy(cfg.port,"/dev/ttyS1"); break;
	}

	//Open port
	cfg.fd_port = open(cfg.port, O_RDWR | O_NOCTTY );
	if (cfg.fd_port <0){
	    return ERROR;
	}

	if ( tcgetattr(cfg.fd_port,&oldtio) == -1){
	    printf("\nError reading port settings.\n");
	    return -1;
	}
}
