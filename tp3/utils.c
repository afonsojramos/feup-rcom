#include <stdio.h>
#include <unistd.h>
#include "defines.h"

extern int DONE;

/**
* implementing state machine
* returns the C in [F|A|C|Bcc|F] - where C can be the SET, the UA, ... @see expecting
* @param fd the serial port file descriptor
* @param expecting the char it is expecting
* @stopAlarm boolean to say whether alarm(0); shloud be called or not
*/
void getCmd(int fd, unsigned char expecting, char stopAlarm){
	unsigned char readChar;
	int state = 0;
  printf("Receiving CMD...\n");
	while (state != 5) {       /* loop for input */
  		read(fd, &readChar, 1);   /* returns after 1 char has been input */
  		printf("read 0x%X state:%d\n", readChar, state);
  		unsigned char packet_A, packet_C;
  		switch (state){
  			case 0:
  				if (readChar==FLAG){
  					state = 1; // this is the beginning of a packet
  				}else
  					state=0;
  			break;
  			case 1:
  				packet_A=readChar; // we received the byte A, here. Storing.
  				if(readChar==A)
  					state = 2;
  				else
  					state = 0;
  			break;
  			case 2:
  				packet_C=readChar; // we received the byte C, here. Storing.
  				if(readChar==expecting)
  					state=3;
  				else
  					state=0;
  			break;
  			case 3:
  				if((packet_A ^ packet_C)==readChar) //expecting the bcc1
  					state=4;
  				else
  					state=0;
  			break;
  			case 4:
  				if(readChar==FLAG){
  					state = 5;
  				}else{
  					state=0;
  				}
  			break;
  		}
    }
    DONE = TRUE;
    if(stopAlarm){
      alarm(0);
    }
    printf("Received CMD properly\n");

}
