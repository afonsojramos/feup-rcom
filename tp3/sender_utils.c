#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define FALSE 0
#define TRUE 1
//llopen defines
#define FLAG 0x7E
#define FLAG_R1 0x7D
#define FLAG_R2 0x5E
#define FLAG_R3 0x5D
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define C_S0 0x00
#define C_S1 0x40


unsigned char SET[5];
unsigned char UA[5];

int attempts = 0;//llopen, llwrite attempt count
int DONE;
int receiver;
unsigned char * I;//trama de informacao a enviar
char CurrentC = C_S0;//the first C sent is C_S0, it will then change on each iteration

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

//send packet, implementing attempts and timeouts
void sendSET(){
  DONE = FALSE;
  if(attempts == 3){
    printf("Failed 3 times, exiting...\n");
    exit(-1);
  }
  printf("Attempt %d/3:\n", (attempts + 1));
  int sentBytes = 0;
  while(sentBytes != 5){
    sentBytes = write(receiver, SET, 5);
    printf("sentBytes: %d\n", sentBytes);
  }
  attempts++;
  if(!DONE){
    alarm(3);
  }else{
    attempts = 0;//reset the attempt count
  }
}

//returns the C in [F|A|C|Bcc|F] - which can be the SET or the UA
void receiveUA(int fd){
  unsigned char c;//last char received
  int state = 0;
  printf("Receiving UA...\n");
  while(state != 5){
    read(fd, &c, 1);
    printf("State %d - char: 0x%X\n", state, c);
    switch (state) {
      case 0://expecting flag
        if(c == UA[0]){
          state = 1;
        }//else stay in same state
        break;
      case 1://expecting A
        if(c == UA[1]){
          state = 2;
        }else if(c != UA[0]){//if not FLAG instead of A
          state = 0;
        }//else stay in same state
        break;
      case 2://Expecting C_SET
        if(c == UA[2]){
          state = 3;
        }else if(c == UA[0]){//if FLAG received
          state = 1;
        }else {//else go back to beggining
          state = 0;
        }
        break;
      case 3://Expecting BCC
        if (c == UA[3]){
          state = 4;
        }else {
          state = 0;//else go back to beggining
        }
        break;
      case 4://Expecting FLAG
        if (c == UA[4]){
          state = 5;
        }else{
          state = 0;//else go back to beggining
        }
        break;
    }
  }
  DONE = TRUE;
  alarm(0);
  printf("Received UA properly\n");
}


/**
* sends a SET to the receiver, implementing timeouts
* @param receiver a file descriptor for the receiver, already open
*/
void llopen(int r){
  receiver = r;
  prepareSet();
  prepareUA();
  printf("Prepared Set and UA\n");
  (void) signal(SIGALRM, sendSET);  // instala rotina que atende interrupcao
  printf("Initial alarm has been set\n");
  sendSET();
  printf("Set has been sent\n");
  receiveUA(receiver);
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
  return 5 + size + countS;

}

/**
* sends a packet of bytes to the receiver, implementing timeouts
* @param receiver a file descriptor for the receiver, already open
* @return -1 if fails or number of bytes written otherwise
*/
int llwrite(int receiver, char * data, int size){
  int sizeToWrite = prepareI(data, size, CurrentC);//loads the data into global I
  for(int i = 0; i <= sizeToWrite; i++){
    printf("%X,", I[i]);
  }
  int written = write(receiver, I, sizeToWrite);
  printf("\nwritten:%d\n", written);
  return -1;
}
