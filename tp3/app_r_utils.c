#pragma once
#include "defines.h"
#include "receiver_utils.c"

typedef struct{
  // if control packet, these are used:
  char* name;
  unsigned int size;
  //break;
  // if data packet, these are used:
  char* content;
  unsigned char nSeq;
  unsigned int cSize;

} rfile;


/* getPacket
Returns valueable info:
 1 - data packet
 2 - start packet
 3 - end packet.

Additionally, return data on the passed variable, rfile* rf:
    if ret==1, then rf->content will be filled with the data packet's content, and should be freed later.
    else, rf->name will be filled with the file's name, and should be freed. The file's size will also be written to rf->size
*/

char getPacket(int fd, rfile* rf){
    char* packet;

    int ret=llreadR(fd, &packet);
    DEBUG_PRINT("[R] llreadR just returned. Printing packet:\n");

    printB(packet, ret);
    if(ret<0){
      return -1; // Something went wrong with the llread.
    }

    // begin state machine to receive C packet
    char state=0, STOP=0;
    int i=-1;
    char control;
    rf->size=0;
    rf->name=NULL;
    while(!STOP){
      i++; //increment packet byte iterator
      if(i>ret){ // if we try to read beyond the end of the packet returned by llread, then something is wrong.
        DEBUG_PRINT("I HIT THE TODO!!\n");
        return control;
      }
      unsigned char l;
      switch(state){
        case 0:
          if(packet[i]!=2 && packet[i]!=3 && packet[i]!=1){
            DEBUG_PRINT("[R] packet is not a control packet. Exiting... \n");
            return packet[i]; // not a control packet.
          }else{
            control=packet[i];
            if(packet[i]==1){ // data packet incoming
              state=4;
            }else{
              state=1;
            }
          }
        break;
        case 1:
          DEBUG_PRINT("[R] reading T byte. i is %d, currentByte is %x\n", i, packet[i]);
          // here we read a T (type) byte, or nothing
          if(i==ret){ // end of packet.
            return control;
          }
          if (packet[i]==0){ // reading file size next.
            state=2;
          }else if(packet[i]==1){ // reading file name next.
            state=3;
          }else{ // undefined behaviour here
            return -5; // received unknown Type packet.
          }
        break;
        case 2: // FILE SIZE
          //we shold be getting a length here.

          l = packet[i];

          char sizeStr[10];
          int j;
          for(j=0;j<=l;j++){
            sizeStr[j]=packet[i++];
          }

          sizeStr[++i]='\0'; // null terminator in string

          rf->size=atoi(sizeStr);
          i-=2; // trolhisse
          DEBUG_PRINT("[R] Just read file size=%d. i is %d\n", rf->size, i);
          state=1;
        break;
        case 3: // FILE NAME
          //we shold be getting a length here.
          ;
          l = packet[i];

          rf->name=malloc(sizeof(char)*(l+1));
          for(int j=0;j<l;j++){
            rf->name[j]=packet[++i]; // copy file name to rf.name
          }
          DEBUG_PRINT("l is %d.\n", l);
          rf->name[l]='\0';
          state=1;
        break;
        case 4:
          rf->nSeq=packet[i]; // get N field of packet and store.
          char tempL2=packet[++i];
          rf->cSize=tempL2*256+packet[++i];
          rf->content=malloc(sizeof(char)*rf->cSize);
          for(int j=0;j<rf->cSize;j++){
            rf->content[j]=packet[++i];
          }

          assert(i==ret); // should be.
          // TODO test this
        break;
      }
    }


    return 0;
}
