#pragma once
#include "defines.h"
#include "receiver_utils.c"

typedef struct{
  char* name;
  unsigned int size;
} rfile;

char get_control(int fd, rfile* rf){
    char* packet;

    int ret=llread(fd, &packet);

    if(ret!=0){
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
        //TODO care comment above
      }
      unsigned char l;
      switch(state){
        case 0:
          if(packet[i]!=2 && packet[i]!=3){
            return -1; // not a control packet.
          }else{
            control=packet[i];
            state=1;
          }
        break;
        case 1:
          // here we read a T (type) byte, or nothing
          if(i==ret){ // end of packet.
            return control;
          }
          if (packet[i]==0){ // reading file size next.
            state=2;
          }else if(packet[i]==1){ // reading file name next.
            state=3;
          }else{ // undefined behaviour here
            return -2; // received unknown Type packet.
          }
        break;
        case 2: // FILE SIZE
          //we shold be getting a length here.
          l = packet[i];
          unsigned char ltemp = l;

          for(int j=0;j<l;j++){
            rf->size |= ( packet[++i] << (ltemp*8)); // TODO TEST THIS VERY THOUROULY
          }
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
          state=1;
        break;
      }
    }


    return 0;
}
