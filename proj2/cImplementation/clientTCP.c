/*      (C)2000 FEUP  */

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
	printf("got code %d\n", code);
	if (code == 421)
	{
		// special termination code.
		exit(-9);
	}
	return code;
}

/**
 * Used to send a command that expects a 200 response.
 * Appends \n to end of command.
 */

int sendGenericCommand(int sockfd, char *cmd)
{
	int bytes;
	char buf[2048];
	strcpy(buf, cmd);
	strcat(buf, "\r\n");

	bytes = write(sockfd, buf, strlen(buf));
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

	int sockfd;
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
	printf("Got IP: %s\n", ip);

	/*server address handling*/
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(21); /*server TCP port must be network byte ordered */

	/*open an TCP socket*/
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
	bzero(buf, 2048);
	strcpy(buf, "USER ");
	strcat(buf, URL.username);
	strcat(buf, "\r\n");
	bytes = write(sockfd, buf, strlen(buf));
	printf("wrote %d bytes.\n", bytes);
	bytes = read(sockfd, buf, 2048);
	if (getCodeFromReply(buf) != 331)
	{
		printf("'USER username' command got reply %sExiting...\n", buf);
		exit(3);
	}
	puts(buf);
	bzero(buf, 2048);
	// send password
	printf("Sending pass\n");
	bzero(buf, 2048);
	strcpy(buf, "PASS ");
	strcat(buf, URL.password);
	strcat(buf, "\r\n");

	bytes = write(sockfd, "PASS sdfw\n", 10);
	bzero(buf, 2048);
	printf("wrote. waiting\n");
	bytes = read(sockfd, buf, 2048);
	printf("received.\n");
	if (getCodeFromReply(buf) != 230)
	{
		printf("'PASS password' command got reply %sExiting...\n", buf);
		exit(3);
	}
	// if we got here, we're good to navigate the server.

	int code = sendGenericCommand(sockfd, "OPTS UTF8 ON");

	printf("code: %d\n", code);

	close(sockfd);
	exit(0);
}
