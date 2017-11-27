#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

unsigned char * packetToSend;//used for the alarm function
int LENGTH;

unsigned char SET[5];
unsigned char UA[5];
unsigned char DISC[5];

extern int DONE;
extern int attempts;

int receiver;
unsigned char * I;//trama de informacao a enviar
char CurrentC = C_S0;//the first C sent is C_S0, it will then change on each iteration
char CurrentREJ = C_REJ0;//the first REJ received is C_REJ1, it will then change on each iteration

void prepareCmd(unsigned char * CMD, unsigned char C){
  CMD[0] = FLAG;
  CMD[1] = A;
  CMD[2] = C;
  CMD[3] = CMD[1] ^ CMD[2];
  CMD[4] = FLAG;
}

char getExpecting(){
  if(CurrentC == C_S0){
    return C_RR1;
  }else{
    return C_RR0;
  }
}
char getExpectingRej(){
  if(CurrentC == C_S0){
    return C_REJ1;
  }else{
    return C_REJ0;
  }
}

void complementCS(){
  if(CurrentC == C_S0){
    CurrentC = C_S1;
  }else{
    CurrentC = C_S0;
  }
}

/**
* send packet, implementing attempts and timeouts
* the global variable char[5] packetToSend must be loaded
*/
void sendPacket(){
  (void) signal(SIGALRM, sendPacket);  // instala rotina que atende interrupcao
  DONE = FALSE;
  if(attempts == 3){
    printf("\nTIMEOUT: Failed 3 times, exiting...\n");
    exit(-1);
  }
  DEBUG_PRINT("Attempt %d/3:\n", (attempts + 1));
  int sentBytes = 0;
  while(sentBytes != LENGTH){
    sentBytes = write(receiver, packetToSend, LENGTH);
    DEBUG_PRINT("sentBytes: %d\n", sentBytes);
  }
  attempts++;
  if(!DONE){
    alarm(3);
  }else{
    attempts = 0;//reset the attempt count
  }
}

/**
* copies the next packet to send to the global variable sourcePacket
* so that the sendPacket function can use ir
*/
void copyToPacketToSend(unsigned char * sourcePacket, int length){
  LENGTH = length;
  for(int i = 0; i < LENGTH; i++){
    packetToSend[i] = sourcePacket[i];
  }
}

void sendWithTimeout(unsigned char * sourcePacket, char expecting, int length){
  packetToSend = (unsigned char *)  malloc(sizeof(unsigned char) * length);
  copyToPacketToSend(sourcePacket, length);
  DEBUG_PRINT("Initial alarm has been set\n");
  sendPacket();
  DEBUG_PRINT("Packet has been sent\n");
  getCmd(receiver, expecting, TRUE);//True means stop alarm after receiving
  free(packetToSend);
}

//send I with timeout, but exepcting RR or REJ, and acting accordingly
void sendIpacketWithTimeout(unsigned char * sourcePacket, int length){
  //preparing packet only happens once
  packetToSend = (unsigned char *)  malloc(sizeof(unsigned char) * length);
  copyToPacketToSend(sourcePacket, length);
  unsigned char readChar = getExpectingRej();//used to verify the receiver response
  do{
    DEBUG_PRINT("Initial alarm has been set\n");
    sendPacket();
    DEBUG_PRINT("Packet has been sent\n");
    //Receiving either RR or REJ
    readChar = getCmdExpectingTwo(receiver, getExpecting(), getExpectingRej(), TRUE);//True means stop alarm after receiving
    if(readChar == getExpectingRej()){
      DEBUG_PRINT("\n\n\n\n\n\n\n\n\n\n\n\n--------------------REJ-----------------------\n\n\n\n\n\n\n\n\n\n\n\n");
    }
  }while(readChar == getExpectingRej());//while rej is received, resend the
  free(packetToSend);
}

/**
* sends a SET to the receiver, implementing timeouts
* @param receiver a file descriptor for the receiver, already open
*/
void llopenS(int r){
  receiver = r;
  prepareCmd(SET, C_SET);
  prepareCmd(UA, C_UA);
  DEBUG_PRINT("Prepared Set and UA\n");
  sendWithTimeout(SET, UA[2], 5);
}

void stuffing(unsigned char * data, int * countS, char byte, int i){
  if(byte == FLAG){//do byte stuffing
    data[4 + i + (*countS)] = FLAG_R1;
    data[4 + i + (++(*countS))] = FLAG_R2;
  }else if(byte == FLAG_R1){//do byte stuffing
    data[4 + i + (*countS)] = FLAG_R1;
    data[4 + i + (++(*countS))] = FLAG_R3;
  }else{
    data[4 + i + (*countS)] = byte;
  }
}

int prepareI(char * data, int size, char C){
  I = malloc(sizeof(unsigned char)*(4 + (size * 2) + 2));//[F|A|C|Bcc1|...Data...|Bcc2|F]
  I[0] = FLAG;
  I[1] = A;
  I[2] = C;
  I[3] = I[1] ^ I[2];
  //initialize Bcc2 for xor
  char bcc2 = data[0];
  I[4] = data[0];
  int countS = 0;//count stuffings
  for(int i = 1; i < size; i++){//iterate data, construct I and calculate Bcc2 simultaneously
    stuffing(I, &countS, data[i], i);
    //I[4 + i + (countS)] = data[i];
    bcc2 = bcc2 ^ data[i];
  }
  stuffing(I, &countS, bcc2, size);
  I[5 + size + countS] = FLAG;
  return 5 + size + countS + 1;

}

/**
* sends a packet of bytes to the receiver, implementing timeouts
* @param receiver a file descriptor for the receiver, already open
* @return -1 if fails or 1 if it is successful
*/
int llwrite(int receiver, char * data, int size){
  DONE = FALSE;
  int sizeToWrite = prepareI(data, size, CurrentC);//loads the data into global I
  sendIpacketWithTimeout(I, sizeToWrite);
  complementCS();
  return DONE;
}

int llcloseS(){
  prepareCmd(DISC, C_DISC);
  DEBUG_PRINT("Prepared DISC\n");
  sendWithTimeout(DISC, DISC[2], 5);
  int sentBytes = 0;
  while(sentBytes != 5){
    sentBytes = write(receiver, UA, 5);
  }
  return 1;
}
