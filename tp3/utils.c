#include <stdio.h>
#include <unistd.h>
#include "defines.h"

int DONE;
int attempts = 0;


#ifdef DEBUG
	#define DEBUG_PRINT(str, ...) (printf("[DEBUG] "), printf(str, ##__VA_ARGS__))
#else
	#define DEBUG_PRINT(str, ...)
#endif


void printB(char* str, unsigned n){
	int i;DEBUG_PRINT(" ");
	for(i=0;i<n;i++){
		#ifdef DEBUG
			printf("%x ", (unsigned char) str[i]);
		#endif
	}
	#ifdef DEBUG
		printf("\n");
	#endif
}

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
	DEBUG_PRINT("Receiving CMD...\n");
	while (state != 5) {       /* loop for input */
  		read(fd, &readChar, 1);   /* returns after 1 char has been input */
  		DEBUG_PRINT("read 0x%X state:%d\n", readChar, state);
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
			DEBUG_PRINT("stopped ALARM\n");
      alarm(0);
	    attempts = 0;//reset the attempt count
    }
    DEBUG_PRINT("Received CMD properly\n");

}


/**
* implementing state machine
* returns the C in [F|A|C|Bcc|F] - where C can be the SET, the UA, ... @see expecting
* @param fd the serial port file descriptor
* @param expecting1 the first char it is expecting
* @param expecting1 the second char it is expecting
* @stopAlarm boolean to say whether alarm(0); shloud be called or not
* @return the expected char that was received
*/
unsigned char getCmdExpectingTwo(int fd, unsigned char expecting1, unsigned char expecting2, char stopAlarm){
	unsigned char readChar, matchExpected = 0;
	int state = 0;
	DEBUG_PRINT("Receiving One of two CMD (0x%x OR 0x%x)...\n", expecting1, expecting2);
	while (state != 5) {       /* loop for input */
  		read(fd, &readChar, 1);   /* returns after 1 char has been input */
  		DEBUG_PRINT("read 0x%X state:%d\n", readChar, state);
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
				if(readChar==expecting1 || readChar == expecting2){
					matchExpected = readChar;//Save the match to return later
  					state=3;
				}else{
					state=0;
				}
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
		DEBUG_PRINT("stopped ALARM\n");
		alarm(0);
		attempts = 0;//reset the attempt count
    }
	DEBUG_PRINT("Received Double exepcting CMD properly: 0x%x\n", matchExpected);
	return matchExpected;
}