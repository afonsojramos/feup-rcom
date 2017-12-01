#pragma once

#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
//c++ includes
#include <string>

#define PORT_FTP 21

using namespace std;

typedef struct _urlParts{
    string username;
    string password;
    string hostname;
    string filepath;
    string filename;
} urlParts;

/**
* parse a given url in the format ftp://[<us>:<password>@]<host>/<url-path>
* @stringUrl the url to parse
* @url the parts of the url are loaded into this variable
* @return returns TRUE on success or FALSE on failure (for instance for invalid urls)
*/
bool loadUrl(string stringUrl, urlParts * url);

#ifdef DEBUG
	#define DEBUG_PRINT(str, ...) (printf("[DEBUG] "), printf(str, ##__VA_ARGS__))
#else
	#define DEBUG_PRINT(str, ...)
#endif
