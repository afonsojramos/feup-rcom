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
	size_t len;
	len=strlen(str);

	for(size_t i = 0; i < len-3; i++){
		if(atoi(str+i)==code && str[i+3]==' ')
		return i;
	}
	return -1;
}

/**
	* We use this function (instead of reading direclty) because there is the
	* possibility that we'll get a multi-line response.

	* return reply code
  */
int getReply(int sockfd){
	char buf[2048];
	int code;

	read(sockfd, buf, 2048);
	if(buf[3]=='-'){
		// multi-line reply here.
		code = getCodeFromReply(buf);
		if(findCmdSpaceInStr(buf, code)>0)
			return code;
		while(1){
			read(sockfd, buf, 2048);
			if(findCmdSpaceInStr(buf, code)>0)
				return code;
		}
	}
	else{
		//single line reply.
		code = getCodeFromReply(buf);
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
  for(int i = 0; i < progresSize; i++){
    printf("%c", '*');
  }
  for(int i = progresSize; i < barSize ; i++){
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
	bytes = read(sockfd, buf, 2048);
	if (getCodeFromReply(buf) == 120)
	{
		// the server is busy. waiting.
		bytes = read(sockfd, buf, 2048);
		if (getCodeFromReply(buf) == 220)
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


	if (getReply(sockfd) != 331)
	{
		printf("'USER username' command got reply %sExiting...\n", buf);
		exit(3);
	}

	// send password
	sprintf(buf, "PASS %s\r\n", URL.password);
	bytes = write(sockfd, buf, strlen(buf));
	if (getReply(sockfd) != 230)
	{
		printf("'PASS password' command got reply %sExiting...\n", buf);
		exit(3);
	}
	// if we got here, we're good to navigate the server.

	//int code = sendGenericCommand(sockfd, "OPTS UTF8 ON");

	// enter passive mode
	sprintf(buf, "PASV\r\n");
	bytes=write(sockfd, buf , strlen(buf));
	bytes=read(sockfd, buf, 2048);
	buf[bytes]=0;
	if (getCodeFromReply(buf) != 227)
	{
		printf("'PASV' command got reply %sExiting...\n", buf);
		exit(3);
	}
	unsigned short passPort = getPASVport(buf);

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
	read(sockfd, buf, 2048);
	size_t filesize;
	sscanf(buf,"%*[^(](%ld bytes", &filesize);

	// we can start reading the file.

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
		displayProgress(rsf, filesize);
	}
	progressEnd();puts("\n");
	//printf("\nftell: %lu. fsize: %lu\n", ftell(f), filesize);
	if((unsigned long)ftell(f)!=filesize){
		printf("File size does not match!\n");
		exit(-6);
	}
	printf("File downloaded successfully!\n");
	fclose(f);
	close(psockfd);
	getReply(sockfd);
	close(sockfd);
	free(buf);
	exit(0);
}
