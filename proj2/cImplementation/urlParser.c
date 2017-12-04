#include "urlParser.h"
#include <stdio.h>
#include <string.h>

parsedURL_t getHostAndPath(char *partOfUrl){
	parsedURL_t ret;
	memset(ret.host, 	 0, 256);
	memset(ret.path, 	 0, 1024);
	memset(ret.filename, 0, 512);

	ret.success = 0;
	char *path;
	if (partOfUrl[0] == '/')
	{
		return ret; // this would mean an empty host.
	}

	path = strstr(partOfUrl, "/");
	if(path==NULL){
		//it looks like the user's trying to download an unexisting file.
		return ret;
	}
	int len=path - (partOfUrl);
	strncpy(ret.host, partOfUrl, len);
	strncpy(ret.path, path+1, 1024); // that +1 skips the slash in the path

	if(strstr(ret.path, "/")==NULL){
		//the file is in the root.
		strncpy(ret.filename, ret.path, 512);
		memset(ret.path, 0, 1024);
	}else{
		char* index=ret.path;
		char* oldIndex;
		while(1){
			oldIndex=index;
			index = strstr(index+1, "/");
			if (index==NULL){
				break;
			}
		}
		// we got the last slash.
		strcpy(ret.filename, oldIndex+1);
		memset(oldIndex+1, 0, oldIndex-ret.path);
	}
	ret.success = 1;
	return ret;
}

parsedURL_t loadUrl(char *url)
{
	// ftp://[<user>:<password>@]<host>/<url-path>

	parsedURL_t ret;
	ret.success = 0;
	memset(ret.protocol, 0, 6);
	memset(ret.username, 0, 256);
	memset(ret.password, 0, 256);
	memset(ret.host, 	 0, 256);
	memset(ret.path, 	 0, 1024);
	memset(ret.filename, 0, 512);
	char *index;
	char *mk1;
	int len;

	index = strstr(url, "://");
	if (index == NULL)
	{
		return ret;
	}

	mk1 = index+3;	   // store marker1
	len = index - url; // length of the protocol
	if (len > 5 || len < 1)
	{
		return ret; // invalid protocol length
	}
	else
	{
		strncpy(ret.protocol, url, len);
	}

	// protocol is good, moving on.

	// let's now look for '@'s. The can be one, but not more!
	index = url;
	int i = -1;
	do
	{
		index++; // forward 1 char
		index = strstr(index, "@");
		i++;
	} while (index != NULL);

	if (i > 0){ // The [<user>:<password>@] block has been specified.
		char uap[512]; // user and pass
		memset(uap, 0, 512); // clear the memory here. Valgrind was complaining. strncpy could fail, you know?
		index = strstr(mk1, "@");
		if(i>1){
			//wait, we have an '@' inside the uap block. It must be part of an email!
			index = strstr(index+1, "@"); //get the index of the second '@'
		}
		len = index - mk1;
		strncpy(uap, mk1, len);
		// ok, we have the <user>:<password> block in the uap variable.
		index=strstr(uap, ":");
		if(index==NULL){
			//no password was provided. We shall use the whole block as a username.
			strcpy(ret.username, uap);
		}else{
			strncpy(ret.username, uap, index-uap);
			strcpy(ret.password, index+1); // skip the ':'
		}
		//update the marker1
		mk1+=strlen(uap)+1;
	}

	parsedURL_t hap = getHostAndPath(mk1);
	if (hap.success == 0)
	{
		return ret;
	}
	else
	{
		strcpy(ret.host, hap.host);
		strcpy(ret.path, hap.path);
		strcpy(ret.filename, hap.filename);
	}
	if(strlen(ret.username)==0){
		strcpy(ret.username, "anonymous");
		strcpy(ret.password, "randompwd");
	}
	ret.success=1;
	return ret;
}
