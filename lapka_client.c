#define _GNU_SOURCE   // For splice
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "protocol.h"

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

		char buf_stdout[1000] = {0};
		read(pipe_stdout[0], buf_stdout, sizeof(buf_stdout) - 1);
		close(pipe_stdout[0]);

		char buf_stderr[1000] = {0};
		read(pipe_stderr[0], buf_stderr, sizeof(buf_stderr) - 1);
		close(pipe_stderr[0]);

		int status;
		waitpid(childpid, &status, 0);

		char result_msg[MAX_STATE];
		snprintf(result_msg, sizeof(result_msg),
				"STDOUT:\n"
				"%s\n"
				"\n"
				"STDERR:\n"
				"%s\n"
				"\n"
				"RETCODE: %i\n",
				buf_stdout, buf_stderr, status
				);
		send_str(sockfd, result_msg);
	}
}
