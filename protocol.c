#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "protocol.h"

void send_str(int sockfd, const char* str)
{
	size_t hostname_len = strlen(str);
	write(sockfd, &hostname_len, sizeof(hostname_len));
	write(sockfd, str, hostname_len);
}

char* recv_str(int sockfd)
{
	size_t len;
	int n = read(sockfd, &len, sizeof(len));
	if (0 == n)
		exit(0);

	if (n < 0) {
		fprintf(stderr, "\n Error : Read len Failed \n");
		exit(1);
	}

	char* str = malloc(len + 1 /* for zero */);
	if (NULL == str) {
		fprintf(stderr, "\n Error : malloc Failed \n");
		exit(1);
	}

	n = read(sockfd, str, len);
	if (0 == n)
		exit(0);

	if (n < 0) {
		fprintf(stderr, "\n Error : Read command Failed \n");
		exit(1);
	}

	str[len] = 0;

	return str;
}
