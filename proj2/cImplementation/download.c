#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "urlParser.h"

int main(int argc, char **argv)
{
	if (argc != 2){
		write(STDOUT_FILENO, "Expected a URL. Please use ./download <FTPurl>\n", 47);
		return -1;
	}

	parsedURL_t URL = loadUrl(argv[1]);

	if (URL.success == 0){
		write(STDOUT_FILENO, "The passed URL was malformed. Please use the following format:\n\tftp://[<user>:<password>@]<host>/<url-path>\n", 109);
	}else{
		printf("protocol: 	%s\n", URL.protocol);
		printf("user:		%s\n", URL.username);
		printf("password:	%s\n", URL.password);
		printf("host:		%s\n", URL.host);
		printf("path:		%s\n", URL.path);
		printf("filename:	%s\n", URL.filename);
	}

	return 0;
}