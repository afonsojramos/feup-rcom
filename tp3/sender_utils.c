#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

//llopen defines
#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

unsigned char SET[5];
unsigned char UA[5];

int attempts = 0;//llopen, llwrite attempt count


void prepareSet(){ //prepare message to send
  SET[0] = FLAG;
  SET[1] = A;
  SET[2] = C_SET;
  SET[3] = SET[1] ^ SET[2];
  SET[4] = FLAG;
}
void prepareUA(){ //prepare message to receive, test agains the one the receiver sends
  UA[0] = FLAG;
  UA[1] = A;
  UA[2] = C_UA;
  UA[3] = UA[1] ^ UA[2];
  UA[4] = FLAG;
}

/**
* sends a SET to the receiver, implementing timeouts
* @param receiver a file descriptor for the receiver, already open
*/
void llopen(int receiver){
  sendSET();
}


/**
* sends a packet of bytes to the receiver, implementing timeouts
* @param receiver a file descriptor for the receiver, already open
*/
void llwrite(int receiver){

}
