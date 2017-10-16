#include <stdio.h>
#include <stdlib.h>
#include "receiver_utils.c"

#define LEN 7


int main(){

	sendUA(stdout);exit(0);
	int i;
	char str[] = {0x14, 0x54, 0x56, 0x74, 0x7d, 0x5e, 0x25};
	printB(str, LEN);


	int n = destuff(str, LEN);

	printB(str, n);
}
