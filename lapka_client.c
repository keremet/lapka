#define _GNU_SOURCE   // For splice
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "protocol.h"

ssize_t read_and_close_pipe(int fd, char* buf, size_t bufsz) {
	ssize_t readed_bytes_all = 0;
	do {
		ssize_t readed_bytes = read(fd, buf + readed_bytes_all, bufsz - readed_bytes_all);
		if (readed_bytes <= 0)
			break;
		readed_bytes_all += readed_bytes;
	} while (readed_bytes_all < bufsz);

	close(fd);
	return readed_bytes_all;
}

int main(int argc, char *argv[]) {
	if (argc != 1 + 1) {
		fprintf(stderr, "\n Usage: %s <ip of server> \n", argv[0]);
		return 1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ((sockfd ) < 0)
	{
		fprintf(stderr, "\n Error : Could not create socket \n");
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(5000)
	};

	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
	{
		fprintf(stderr, "\n inet_pton error occured\n");
		return 1;
	}

	char hostname[MAX_HOSTNAME + 1];
	if (gethostname(hostname, sizeof(hostname)-1) != 0)
	{
		fprintf(stderr, "\n Error : gethostname Failed \n");
		return 1;
	}
	hostname[sizeof(hostname)-1] = 0;

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "\n Error : Connect Failed \n");
		return 1;
	}

	send_str(sockfd, hostname);

	for(;;) {
		size_t sz;
		int n = read(sockfd, &sz, sizeof(sz));
		if (0 == n)
			exit(0);

		if (n < 0) {
			fprintf(stderr, "\n Error : Read len Failed \n");
			exit(1);
		}

		int   pipe_stdin[2];  pipe(pipe_stdin);
		int   pipe_stdout[2]; pipe(pipe_stdout);
		int   pipe_stderr[2]; pipe(pipe_stderr);
		pid_t childpid = fork();
		if (childpid < 0) {
			perror("fork");
			return 1;
		}

		if (0 == childpid) {
			close(pipe_stdin[1]);
			close(pipe_stdout[0]);
			close(pipe_stderr[0]);
			dup2(pipe_stdin[0], 0);
			dup2(pipe_stdout[1], 1);
			dup2(pipe_stderr[1], 2);
			execl("/bin/bash", "/bin/bash", NULL);
		}

		close(pipe_stdin[0]);
		close(pipe_stdout[1]);
		close(pipe_stderr[1]);

		splice(sockfd, NULL, pipe_stdin[1], NULL, sz, 0);
		close(pipe_stdin[1]);

		char result_msg[MAX_STATE];
		char* p_result_msg = result_msg;
		strcpy(p_result_msg, "STDOUT:\n"); p_result_msg += 8;
		p_result_msg += read_and_close_pipe(pipe_stdout[0], p_result_msg, MAX_STDOUT);
		strcpy(p_result_msg, "\n\nSTDERR:\n"); p_result_msg += 10;
		p_result_msg += read_and_close_pipe(pipe_stderr[0], p_result_msg, MAX_STDERR);

		int status;
		waitpid(childpid, &status, 0);
		sprintf(p_result_msg,
				"\n\nRETCODE: %i\n",
				status
				);
		send_str(sockfd, result_msg);
	}
}
