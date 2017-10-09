#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07


char getCmd(int fd){
	char c;
	char ret;
	int state=0;
	int BCC_OK;
	while (1) {       /* loop for input */
		read(fd, &c, 1);   /* returns after 1 char has been input */
		//printf("read %x state:%d\n", c, state);
		char packet_A, packet_C;
		switch (state){
			case 0:
				if (c==FLAG){
					state = 1; // this is the beginning of a packet
				}else
					state=0;
			break;
			case 1:
				packet_A=c; // we received the byte A, here. Storing.
				if(c==A)
					state = 2;
				else
					state = 0;
			break;
			case 2:
				packet_C=c; // we received the byte C, here. Storing.
				if(c==C_SET)
					state=3;
				else
					state=0;
			break;
			case 3: 
				if((packet_A ^ packet_C)==c)
					BCC_OK=1;
				else
					BCC_OK=0;

				if(BCC_OK)
					state=4;
				else
					state=0;
			break;
			case 4:
				if(c==FLAG){
					// SUCCESS!!
					ret=packet_C;
					return ret;
				}else{
					state=0;
				}			
			break;

		}	
    }
	return -1;
}

int sendUA(int fd){ //prepare message

	unsigned char UA[5];

	UA[0] = FLAG;
	UA[1] = A;
	UA[2] = C_UA;
	UA[3] = UA[1] ^ UA[2];
	UA[4] = FLAG;

	return write(fd, UA, 5);
}

char llopen(int fd){
	
	char cmd = getCmd(fd); // Open fd 
	printf("We got a %x!\n", cmd);
	int retUA=sendUA(fd);
	if(retUA==-1){
		return retUA;
	}
	return 0;
}


int llread(int fd, char* dest){

	int rsf=0; // (bytes) read so far
	int cbs=100; // current buffern size

	/* Allocate memory for writing Data Packets*/
	dest=malloc(sizeof(char)*cbs);
	if(dest==NULL){
		perror("malloc");
	}

	/* Begin state machine */
	char c;
	int state=0;
	int BCC_OK, STOP=0;
	while (STOP==0) {       /* loop for input */
		read(fd, &c, 1);   /* returns after 1 char has been input */
		printf("read %x state:%d\n", c, state);
		char packet_A, packet_C;
		switch (state){
			case 0:
				if (c==FLAG){
					state = 1; // this is the beginning of a packet
				}else
					state=0;
			break;
			case 1:
				packet_A=c; // we received the byte A, here. Storing.
				if(c==A)
					state = 2;
				else
					state = 0;
			break;
			case 2:
				packet_C=c; // we received the byte C, here. Storing.
				if(c==C_SET)
					state=3;
				else
					state=0;
			break;
			case 3: 
				if((packet_A ^ packet_C)==c)
					BCC_OK=1;
				else
					BCC_OK=0;

				if(BCC_OK)
					state=4;
				else
					state=0;
			break;
			case 4:
				if(c==FLAG){
					// end of datagram!!
					STOP=1;
				}else{
					// we're receiving a data byte. Storing...
					if(rsf==cbs){ // ou buffer is full. Allocating more space...
						cbs*=1.5;
						dest=realloc(dest, sizeof(char)*cbs);
						if(dest==NULL){
							perror("realloc");
						}
					}
					dest[rsf++]=c;
				}			
			break;

		}	
    }

	
	// having read the whole datagram, ne now need to destuff it.

/*	int i;
	for(i=0;i<rsf;i++){
		printf("c: %c, x: %x\n", dest[i], dest[i]);	
	}
*/

	return 0;	
}
