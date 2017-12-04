#pragma once

typedef struct
{
	char success;
	char protocol[6];
	char username[256];
	char password[256];
	char host[256];
	char path[1024];
	char filename[512];
} parsedURL_t;

parsedURL_t loadUrl(char *url);