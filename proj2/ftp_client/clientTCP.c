#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include "clientTCP.h"
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>


size_t min(size_t a, size_t b){
	if(a<b)
		return a;
	return b;
}

/**
	* We use this function because different servers reply in different ways.
	* param buf pointer to the string to parse.
	* param fsize pointer to where the size should be written (in bytes)
	* return 1 if fsize is exact, 0 otherwise, and -1 if fails (fsize becomes 0)
  */
char getFizeSizeFrom150(char* buf, size_t* fsize){
	/**
		* replies may come in many formats:
		* - 159845 bytes
		*	- 5463 kbytes
		*/
		char precision=0;
		/* first fing to do is determine the integer (or float)
		associated with the file size. */
		char* sizeStr;
		printf("str is %s\n", buf);

		sizeStr = strstr(buf, "bytes");
		// find bytes in string.
		if(sizeStr==NULL){// aka didn't find
			*fsize=0;
			return -1;
		}
		sizeStr-=2; // sizeStr should start at least 2 chars before "bytes".
		while(1){
			// go back while we have numbers (not YOLO cause we'll do some checking).
			if( (('0' <=*(sizeStr-1) && *(sizeStr-1)<='9' )||*(sizeStr-1)=='.') && sizeStr-1>buf){
				sizeStr--;
			}else{
				break;
			}
		}

		printf("guessing size is near %s\n", sizeStr);

		//ok, we still need to check if we're talking bytes, kbytes, or something else.

		char* multipier = strchr(sizeStr, ' ');

		unsigned long mult=0;
		printf("switching on %c\n", *(multipier+1));
		switch (*(multipier+1)) {
			case 'b': // b is the first char of Bytes.
				mult=1;
				precision=1;
			break;
			case 'k': // k stands for kilo.
				mult=1000;
			break;
			default:
				// ups, unrecognized multiplier.e  function  fread()  reads nmemb items of data, each size bytes long, from the stream pointed to by stream, storing them at the location
       given by ptr.

				*fsize=0;
				return -1;
			break;
		}

		*fsize=(unsigned int)(mult*atof(sizeStr)); // store fsize

		return precision;
}


int getCodeFromReply(char *str)
{
	int code;
	code = atoi(str);
	if (code == 421){
		// special termination code.
		exit(-9);
	}
	return code;
}

size_t getPASVport(char* cmd){
	int p1, p2;
	char* index = strchr(cmd, '(');
	if(index==NULL){
		exit(-8);
	}

	sscanf(index, "(%*d, %*d, %*d, %*d, %d, %d)", &p1, &p2);
	return p1*256+p2;
}

long findCmdSpaceInStr(char* str, int code){
	size_t len, i;
	len=strlen(str);
	for(i = 0; i < len-3; i++){
		if(atoi(str+i)==code && str[i+3]==' ')
		return i;
	}
	return -1;
}

/**
	* We use this function (instead of reading direclty) because there is the
	* possibility that we'll get a multi-line response.
	* param sockfd the file descriptor of the socket from which we'll read.
	* param strRet if NULL, no string is passed, else memory is alloc'ed and string is written there
	* return reply code
  */
int getReply(int sockfd, char** strRet){
	if(strRet!=NULL){
		(*strRet)=malloc(2048*sizeof(char));
		bzero(*strRet, 2048);
	}
	int code;
	int bytes;
	char buf[2048];
	bzero(buf, 2048);
	bytes = read(sockfd, buf, 2048);
	//printf("read %d bytes: %s\n", bytes, buf);
	if(buf[3]=='-'){
		// multi-line reply here.
		code = getCodeFromReply(buf);
		int wsf=bytes; // written so far
		if(findCmdSpaceInStr(buf, code)>=0){
			if(strRet!=NULL){
				strncat(*strRet, buf, 2048-wsf);
				wsf+=bytes;
			}
			return code;
		}
		while(1){
			bytes = read(sockfd, buf, 2048);
			if(strRet!=NULL){
				strncat(*strRet, buf, 2048-wsf);
				wsf+=bytes;
			}
			if(findCmdSpaceInStr(buf, code)>=0)
				return code;
		}
	}
	else{
		//single line reply.
		code = getCodeFromReply(buf);
		if(strRet!=NULL)
			strncpy(*strRet, buf, 2048); // write reply to strRet if it is not NULL
	}
	return code;
}


void progressInit(){
		printf("\033[H\033[J");
		//fflush(stdout);
		printf("\e[?25l");
}
void progressEnd(){
	printf("\e[?25h\n");
}

void displayProgress(unsigned int currentSize, unsigned int totalSize){
	static int barSize = 40;
  double progress = (((double)currentSize)/totalSize)*100;
  int progresSize = (int)((progress * barSize)/100);
	//printf("%d\n", ((progresSize - lastProgressSize)));

	printf("\033[A\r[");
  int i;  
  for(i = 0; i < progresSize; i++){
    printf("%c", '*');
  }
  for(i = progresSize; i < barSize ; i++){
    printf("%c", '.');
  }
  printf("] %2.2f%%", progress);
	fflush(stdout);
}


/**
 * Used to send a command that expects a 200 response.
 * Appends \r\n to end of command.
 */

int sendGenericCommand(int sockfd, char *cmd)
{
	int bytes;
	char buf[2048];
	strcpy(buf, cmd);
	strcat(buf, "\r\n");

	bytes = write(sockfd, buf, strlen(buf));
		if(bytes<0)	return -1; //couldn't write
	bytes = read(sockfd, buf, 2048);
	int retcode;
	retcode = getCodeFromReply(buf);
	if (retcode != 200)
	{
		return retcode;
	}
	else
		return 0;
}

char getFileFromFTPServer(parsedURL_t URL)
{

	int psockfd, sockfd;
	struct sockaddr_in server_addr;
	int bytes;
	char *buf;
	struct addrinfo hints, *infoptr;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // AF_INET means IPv4 only addresses

	int result = getaddrinfo(URL.host, NULL, &hints, &infoptr);
	if (result)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
		exit(1);
	}

	char ip[64];
	getnameinfo(infoptr->ai_addr, infoptr->ai_addrlen, ip, sizeof(ip), NULL, 0, NI_NUMERICHOST);

	freeaddrinfo(infoptr);

	printf("IP: %s\n", ip);
	printf("Press ENTER to continue...\n");
	getc(stdin);
	/*server address handling*/
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(21); /*server TCP port must be network byte ordered */

	/*open a TCP socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		exit(0);
	}
	/*connect to the server*/
	if (connect(sockfd,
				(struct sockaddr *)&server_addr,
				sizeof(server_addr)) < 0)
	{
		perror("connect()");
		exit(0);
	}
	/*get a string from the server*/

	buf = malloc(2048 * sizeof(char));
	if (getReply(sockfd, NULL) == 120)
	{
		// the server is busy. waiting.
		if (getReply(sockfd, NULL) == 220)
		{
			// carry on
		}
		else
		{
			//weird behaviour.
			exit(1);
		}
	}
	//ready to send user name
	sprintf(buf, "USER %s\r\n", URL.username);
	bytes = write(sockfd, buf, strlen(buf));


	if (getReply(sockfd, NULL) != 331)
	{
		printf("'USER username' command got reply %sExiting...\n", buf);
		exit(3);
	}

	// send password
	sprintf(buf, "PASS %s\r\n", URL.password);
	bytes = write(sockfd, buf, strlen(buf));
	if (getReply(sockfd, NULL) != 230)
	{
		printf("'PASS password' command got reply %sExiting...\n", buf);
		exit(3);
	}
	// if we got here, we're good to navigate the server.

	//int code = sendGenericCommand(sockfd, "OPTS UTF8 ON");

	// enter passive mode
	sprintf(buf, "PASV\r\n");
	bytes=write(sockfd, buf , strlen(buf));
	char* retStr;

	buf[bytes]=0;
	if (getReply(sockfd, &retStr) != 227)
	{
		printf("'PASV' command got reply %sExiting...\n", buf);
		exit(3);
	}
	printf("getReply wrote to retStr: %s\n", retStr);

	unsigned short passPort = getPASVport(retStr);
	free(retStr);

	printf("Passive mode port is %d.\n", passPort);

	/*server address handling*/
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(passPort); /*server TCP port must be network byte ordered */

	/*open a TCP socket*/
	if ((psockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		exit(0);
	}
	/*connect to the server*/
	if (connect(psockfd,
				(struct sockaddr *)&server_addr,
				sizeof(server_addr)) < 0)
	{
		perror("connect()");
		exit(0);
	}

		// we should now change directory, if necessary
	if(strlen(URL.path) !=0){
		char cmd[1512];
		sprintf(cmd, "CWD %s", URL.path);
		sendGenericCommand(sockfd, cmd);
	}else{
		sendGenericCommand(sockfd, "CWD /");
	}
	sendGenericCommand(sockfd, "TYPE I");

	bzero(buf, 2048);
	sprintf(buf, "RETR %s\r\n", URL.filename);
	bytes=write(sockfd, buf, strlen(buf));
	getReply(sockfd, &retStr);
	size_t filesize;
	char precise;
	precise = getFizeSizeFrom150(retStr, &filesize);
	free(retStr);
	size_t rsf=0; // read so far
	FILE* f;
	progressInit();
	f=fopen(URL.filename, "w"); // get FILE*
	while(1){
		bytes=read(psockfd, buf, 2048);
		if(bytes<=0)
			break;
		rsf+=bytes;
		fwrite(buf, bytes, 1, f);
		if(precise !=-1) // -1 means failure, and fsize came out 0. No progress bar then.
			displayProgress(min(rsf, filesize), filesize);
		//printf("progress: %ld/%ld\n", rsf, filesize);
	}
	progressEnd();
	//printf("\nftell: %lu. fsize: %lu\n", ftell(f), filesize);
	if(precise){
		//then we can do this checking.
		if((unsigned long)ftell(f)!=filesize){
			printf("File size does not match!\n");
			exit(-6);
		}
	}

	printf("File downloaded successfully!\n");
	fclose(f);
	close(psockfd);
	getReply(sockfd, NULL);
	close(sockfd);
	free(buf);
	exit(0);
}
