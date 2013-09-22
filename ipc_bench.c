/**
 * Copyright (c) 2013, University of Virginia
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "bench_op.h"

#define MSG_LEN 512

static int iter = 1000000;
static pid_t child = -1;
static int fds[2];
static char msg[MSG_LEN];
static int sock = -1, client_sock = -1;
static const char *sock_path = "/tmp/ipc_bench.sock";

static inline void ipc_bench_signal() {
	kill(child, SIGTERM);
	wait(NULL);
}

static inline void ipc_bench_pipe() {
	write(fds[1], msg, MSG_LEN);
}

static inline void ipc_bench_socket() {
	write(client_sock, msg, MSG_LEN);
	read(client_sock, msg, MSG_LEN);
}

void usage(const char *command) {
	fprintf(stderr, "Usage: %s [-i <iterations>]\n", command);
}

int main(int argc, char *argv[]) {
	int ch;

	while ((ch = getopt(argc, argv, "i:h?")) != -1) {
		switch (ch) {
			case 'i':
				iter = atoi(optarg);
				break;

			case '?':
			case 'h':
				usage(argv[0]);
				break;

			default:
				break;
		}
	}

	pid_t pid = fork();
	if (pid > 0) {
		child = pid;
		BENCH_OP(ipc_bench_signal);
	} else if (pid == 0) {
		// wait to be killed	
	} else {
		perror("fork");
		return -1;
	}

	if (pipe(fds) < 0) {
		perror("pipe");
		return -1;
	}

	pid = fork();
	if (pid > 0) {
		close(fds[0]);
		BENCH_OP(ipc_bench_pipe);
		close(fds[1]);
		wait(NULL);
	} else if (pid == 0) {
		close(fds[1]);
		while (read(fds[0], msg, MSG_LEN)) ;
		close(fds[0]);
		exit(0);
	} else {
		perror("fork");
		return -1;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_path, strlen(sock_path));

	pid = fork();
	if (pid > 0) {
		if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return -1;
		}

		unlink(sock_path);

		if (bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0) {
			perror("bind");
			return -1;
		}

		if (listen(sock, 1) < 0) {
			perror("listen");
			return -1;
		}

		if ((client_sock = accept(sock, NULL, NULL)) < 0) {
			perror("accept");
			return -1;
		}

		BENCH_OP(ipc_bench_socket);

		close(client_sock);
		close(sock);

		unlink(sock_path);

		wait(NULL);
	} else if (pid == 0) {
		if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return -1;
		}

		int retry = 10;

		while (retry--) {
			if (connect(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == 0) {
				break;
			}

			sleep(1);
		}

		if (retry < 0) {
			perror("connect timeout");
		}

		while (read(sock, msg, MSG_LEN) > 0) {
			write(sock, msg, MSG_LEN);
		}

		close(sock);

		exit(0);
	} else {
		perror("fork");
		return -1;
	}
}

