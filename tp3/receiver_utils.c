#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#ifdef DEBUG
	#define DEBUG_PRINT(str, ...) printf(str, ##__VA_ARGS__)
#else
	#define DEBUG_PRINT(str, ...)
#endif

#define c2Bit(x)	(x&0b01000000)>>6

char inline comp(char x){
	if(x==1){
		return 0;
	} else {
		return 1;
	}
}

void printB(char* str, unsigned n){
	int i;
	for(i=0;i<n;i++){
		printf("%x ", (unsigned char) str[i]);
	}
	printf("\n");
}


int sendSU(int fd, char control, char flag){
	printf("%d", flag);
	//take care of the flag, if need be.
	if((control==RR_0||control==C_REJ)&&flag!=0){
		control|=1<<7;
	}
	unsigned char cmd[5];

	cmd[0] = FLAG;				// ┎ start flag
	cmd[1] = A;					// ┃ address field
	cmd[2] = control;			// ┃ control field
	cmd[3] = cmd[1] ^ cmd[2];	// ┃ BCC1
	cmd[4] = FLAG;				// ┖ end flag

	//printB((char*) cmd, 5);
	return write(fd, cmd, 5);
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
				fprintf(stderr, "I got an unrecognized escape seq. Rejecting.\n");
				// we got an unrecognized escape seq. returning error.
				return -1;
			}
		}
	}

	return n;
}

char llopen(int fd){
	getCmd(fd, C_SET, FALSE); //TODO: make C_SET a thing

	int retUA=sendSU(fd, C_UA, 0);//TODO: make this accessible
	if(retUA==-1){
		return retUA;
	}
	return 0;
}


int llread(int fd, char* dest){
	DEBUG_PRINT("[DEBUG] Entered llread.\n");
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
	char packet_A, packet_C;
	while (STOP==0) {       /* loop for input */
		//printf("waiting for input...\n");
		read(fd, &c, 1);   /* returns after 1 char has been input */
		//printf("read %x state:%d\n", c, state);
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

	if(n<0){
		//The destuff function didn't like the body passed. We should reject.
		sendSU(fd, C_REJ, 1);
	}

	// Time to check our BCC2
	int i;
	unsigned char BCC2=0x00;

	for(i=0; i<n-1;i++){ // until n-2 because n	-1 is the BCC2 itself.
		BCC2^=dest[i];
	}

	DEBUG_PRINT("[DEBUG] BCC2 test: %x==%x?\n", BCC2, dest[n-1]);
	if(BCC2 != dest[n-1]){
		//BCC2 check failed!
		DEBUG_PRINT("[DEBUG]\t\t\tFAILED!\n");
		sendSU(fd, C_REJ, 1);
	}else{
		// Getting this frame was an absolute success! Acknowledging!
		DEBUG_PRINT("[DEBUG] sending RR(%d).\n", c2Bit(packet_C));
		sendSU(fd, C_REJ, comp(c2Bit(packet_C)));
	}

	return 0;
}
