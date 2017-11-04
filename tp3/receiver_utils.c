#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "utils.c"

#include <sys/time.h>  // Time-tests


#define c2Bit(x)	(x&0b01000000)>>6

struct tProp{
	unsigned long acc;
	unsigned long start;
	unsigned int count;
};

struct tProp TT_tprop = { .acc = 0, .start = 0, .count = 0 };


void TT_iReceived(){
	struct timeval timecheck;

    gettimeofday(&timecheck, NULL);
    TT_tprop.start = (unsigned long)timecheck.tv_sec * 1000000 + (unsigned long)timecheck.tv_usec;
	
}


void TT_rrSent(){
	struct timeval timecheck;
	
	unsigned long currentTime;

    gettimeofday(&timecheck, NULL);
    currentTime = (unsigned long)timecheck.tv_sec * 1000000 + (unsigned long)timecheck.tv_usec;
	
	TT_tprop.acc+=currentTime-TT_tprop.start;
	TT_tprop.count++;

}


void TT_printStats(){
	printf("======== Transmission Statictics ========\n");
	printf("| total time waiting: %05lu		  |",		 	 TT_tprop.acc );
	printf("| Packets received: %05u			  |", 		 TT_tprop.count );
	printf("| Average Tprop: %05lu			  |", 			(TT_tprop.acc/2)/TT_tprop.count );
}

char comp(char x){
	if(x==1){
		return 0;
	} else {
		return 1;
	}
}


int sendSU(int fd, char control){


	unsigned char cmd[5];

	cmd[0] = FLAG;				// ┎ start flag
	cmd[1] = A;					// ┃ address field
	cmd[2] = control;			// ┃ control field
	cmd[3] = cmd[1] ^ cmd[2];	// ┃ BCC1
	cmd[4] = FLAG;				// ┖ end flag

	printB((char*) cmd, 5);
	
	if(control == C_RR0 || control == C_RR1){
		TT_rrSent();
	}
	return write(fd, cmd, 5);
}


int destuff(char* str, unsigned int n){
	/*

	0x7e is stuffed to 0x7d 0x5e
	0x7d is stuffed to 0x7d 0x5d
	*/
	DEBUG_PRINT("destuffing function got n=%d\n", n);
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
	DEBUG_PRINT("destuffing function returning n=%d\n", n);
	return n;
}

char llopenR(int fd){
	getCmd(fd, C_SET, FALSE);

	int retUA=sendSU(fd, C_UA);
	if(retUA==-1){
		return retUA;
	}
	return 0;
}


int llread(int fd, char** remote_dest){
	char* dest=*remote_dest;

	DEBUG_PRINT("Entered llread.\n");
	int rsf=0; // (bytes) read so far
	int cbs=100; // current buffern size

	/* Allocate memory for writing Data Packets*/
	dest=malloc(sizeof(char)*(cbs+1));
	if(dest==NULL){
		perror("malloc");
	}
	/* Begin state machine */
	unsigned char c;
	int state=0;
	unsigned char BCC_OK, STOP=0;
	char packet_A, packet_C;
	while (STOP==0) {       /* loop for input */
		//printf("waiting for input...\n");
		read(fd, &c, 1);   /* returns after 1 char has been input */
		DEBUG_PRINT("read %x state:%d\n", c, state);
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
				else if(c==FLAG)
					state = 1;
				else
					state = 0;
			break;
			case 2:
				packet_C=c; // we received the byte C, here. Storing.
				if(c==C_S0 || c==C_S1)
					state=3;
				else if(c==C_DISC)
					state=5;
				else
					state=0;
			break;
			case 3:
				if((packet_A ^ packet_C)==c)
					BCC_OK=1;
				else
					BCC_OK=0;

				if(BCC_OK){
					state=4;
					TT_iReceived();
				}else
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
			case 5:
				if((packet_A ^ packet_C)==c)
					BCC_OK=1;
				else
					BCC_OK=0;

				if(BCC_OK){
					state=6;

				}else
					state=0;
			break;
				case 6:
					if(c==FLAG){
						// end of datagram!! We got a DISC though.
						STOP=1;
						sendSU(fd, C_DISC);
						TT_printStats();
						getCmd(fd, C_UA, FALSE);
						free(dest);
						return -6; // signal DISC
						// Do not attmpt to read from remote_dest after returning,
						//for it will most certainly not be set.
					}
				break;

		}
    }


	// having read the whole datagram, ne now need to destuff it.

	int n = destuff(dest, rsf);

	printB(dest, n);

	if(n<0){
		//The destuff function didn't like the body passed. We should reject.
		DEBUG_PRINT("Error occurred during destuffing. Sending C_REJ(%d).\n", comp(c2Bit(packet_C)));
		if(comp(c2Bit(packet_C))==0){
			sendSU(fd, C_REJ0);
		}else{
			sendSU(fd, C_REJ1);
		}
	}

	// Time to check our BCC2
	int i;
	unsigned char BCC2=0x00;

	for(i=0; i<n-1;i++){ // until n-2 because n	-1 is the BCC2 itself.
		BCC2^=dest[i];
	}

	DEBUG_PRINT("BCC2 test: %x==%x?\n", BCC2, (unsigned char) dest[n-1]);
	if(BCC2 !=  (unsigned char) dest[n-1]){
		//BCC2 check failed!
		DEBUG_PRINT("\t\tFAILED!\n");
		//DEBUG_PRINT("Waiting... ");
		//sleep(2);
		if(comp(c2Bit(packet_C))==0){
			sendSU(fd, C_REJ0);
		}else{
			sendSU(fd, C_REJ1);
		}
		printf("rej sent.\n");
		return -15;
	}else{
		// Getting this frame was an absolute success! Acknowledging!
		DEBUG_PRINT("sending RR(%d).\n", comp(c2Bit(packet_C)));
		if(comp(c2Bit(packet_C))==0){
			sendSU(fd, C_RR0);
		}else{
			sendSU(fd, C_RR1);
		}

	}

	*remote_dest=dest; // correct the parameter pointer
	DEBUG_PRINT("n at end of llread: %d\n", n-1); //excluding BCC2
	return n-1;
}


unsigned char llcloseR(){
	return 0;
}
