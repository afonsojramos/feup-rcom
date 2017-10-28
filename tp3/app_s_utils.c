#pragma once
#include "defines.h"
#include "sender_utils.c"

int sendData(int fd, char* bytes, unsigned int n);

int sendControl(int fd, unsigned char start, char *fileName, unsigned int fileSize)
{
    char bytes[600]; // controlPackets should never be bigger than 1+2*(1+255)=513 bytes
    if(start==TRUE){
        bytes[0]=2; // START CONTROL PACKET
    }else{
        bytes[0]=3; // END CONTROL PACKET
    }
    bytes[1]=0;

    char sizeStr[10];

    sprintf(sizeStr, "%d", fileSize);

    unsigned char len; // this will be used to hold strlens

    len=strlen(sizeStr);
    bytes[2]=len; // pass file size string length here
    memcpy(bytes+3, sizeStr, len); // copy file size string to bytes to send;

    unsigned int currPos=3+len; // make a currPos variable to keep track of our current position.

    bytes[currPos]=1; // here we signal TYPE = 1 (file name)
    len=strlen(fileName);
	DEBUG_PRINT("file name len=%d\n", len);
    bytes[++currPos]=len; // pass file name string length here
    memcpy(bytes+currPos+1, fileName, len); // copy file name string to bytes to send.

    currPos+=len+1; // update currPos
	printB(bytes, currPos);
    // HOORAY -- our packet is complete! Sending...

    return llwriteS(fd, bytes, currPos);
}
