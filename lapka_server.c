#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <threads.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "protocol.h"

int process_connection (int connfd) {
	size_t hostname_len;
	int n = read(connfd, &hostname_len, sizeof(hostname_len));
	if (n <= 0 || hostname_len > MAX_HOSTNAME) {
		fprintf(stderr, "\n Error : Read hostname_len Failed \n");
		return 1;
	}

	char hostname[hostname_len + 1];
	read(connfd, hostname, hostname_len);
	hostname[hostname_len] = 0;

	printf("%s\n", hostname);

	DIR *d_command = opendir("commands");
	if (NULL == d_command) {
		fprintf(stderr, "Cannot open the command catalog\n");
		return 1;
	}
	for (struct dirent* de; NULL != (de = readdir(d_command));) {
		if ('.' == de->d_name[0])
			continue;

		char fn_state[PATH_MAX];
		snprintf(fn_state, sizeof(fn_state), "commands/%s/state/%s", de->d_name, hostname);
		if (access(fn_state, F_OK) == 0)
			continue;

		char fn_command[PATH_MAX];
		snprintf(fn_command, sizeof(fn_command), "commands/%s/command", de->d_name);
		int fd_command = open(fn_command, O_RDONLY);
		if (fd_command < 0) {
			fprintf(stderr, "Cannot open file %s\n", fn_command);
			continue;
		}
		struct stat st;
		fstat(fd_command, &st);
		size_t sz = st.st_size;
		write(connfd, &sz, sizeof(sz));
		sendfile(connfd, fd_command, NULL, sz);
		close(fd_command);

		size_t state_len;
		int n = read(connfd, &state_len, sizeof(state_len));
		if (n <= 0 || state_len > MAX_STATE) {
			fprintf(stderr, "\n Error : Read state_len Failed \n");
			return 1;
		}

		char dir_state[PATH_MAX];
		snprintf(dir_state, sizeof(dir_state), "commands/%s/state", de->d_name);
		if (access(dir_state, F_OK) != 0)
			mkdir(dir_state, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

		int fd_state = open(fn_state, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if (fd_state < 0) {
			fprintf(stderr, "Cannot open file %s\n", fn_state);
			continue;
		}

		char state[state_len];
		n = read(connfd, state, state_len);
		if (n <= 0 || state_len > MAX_STATE) {
			fprintf(stderr, "\n Error : Read state_len Failed \n");
			return 1;
		}
		write(fd_state, state, state_len);
		close(fd_state);
	}
	closedir(d_command);

	close(connfd);
	return 0;
}

int main(int argc, char *argv[]) {
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
		fprintf(stderr, "setsockopt error\n");
		return 1;
	}
	if (bind(listenfd,
		(struct sockaddr*)&((struct sockaddr_in) {
			.sin_family = AF_INET,
			.sin_addr.s_addr = htonl(INADDR_ANY),
			.sin_port = htons(5000)
		}), 
		sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "Bind error\n");
		return 1;
	}

	listen(listenfd, 10);

	for(;;) {
		int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

		thrd_t thr;
		thrd_create(&thr, (thrd_start_t)process_connection, (void*)connfd);
		thrd_detach(thr);
	}
}
