#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

#define FLAG_R1 0x7D
#define FLAG_R2 0x5E
#define FLAG_R3 0x5D

#define C_S0 0x00
#define C_S1 0x40


void printB(char* str, unsigned n){
	int i;	
	for(i=0;i<n;i++){
		printf("%x", str[i]);		
	}
	printf("\n");
}

int destuff(char* str, unsigned int n){
	/*

	0x7e is stuffed to 0x7d 0x5e
	0x7d is stuffed to 0x7d 0x5d	
	*/

	int i,j;
	for(i=0;i<n;i++){
		if(str[i]==FLAG_R1){
			if (str[i+1]==FLAG_R2){ // replace with 0x7e
				str[i]=FLAG;
				for(j=i+1;j<=n;j++){
					str[j]=str[j+1];
				}
				n--;
			}
			else if (str[i+1]==FLAG_R3){ // replace with 0x7d
				str[i]=FLAG_R1;
				for(j=i+1;j<=n;j++){
					str[j]=str[j+1];
				}
				n--;
			}else{
				fprintf(stderr, "I got an unrecognized escape seq. Exiting...\n");
				assert(0); // we got an unrecognized escape seq. // TODO Send REJ
			}
		}
	}

	return n;
}

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

	printB((char*) UA, 5);

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
	printf("Entered llread.\n");
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
		//printf("waiting for input...\n");
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
				if(c==C_S0 || c==C_S1)
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

	int n = destuff(dest, rsf);

	// Time to check our BCC2
	int i;
	unsigned char BCC2=0x00;

	for(i=0; i<n-1;i++){ // until n-2 because n	-1 is the BCC2 itself.
		BCC2^=dest[i];
	}
	//printf("%x==%x?\n\n", BCC2, dest[n-1]);
	if(BCC2 != dest[n-1]){
		//BCC2 check failed!
		// TODO Send REJ
	}else{
		// Getting this frame was an absolute success! Acknowledging!

		// TODO send RR
	}
	printB(dest, n);
	
	return 0;	
}
