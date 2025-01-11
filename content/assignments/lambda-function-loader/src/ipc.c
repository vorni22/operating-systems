// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <errno.h>

#include "ipc.h"

int curr_id = 0;

struct sockaddr_un get_sockaddr(const char *path) {
	struct sockaddr_un addr;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, strlen(path) + 1, "%s", path);

	return addr;
}

int create_listener() {
	struct sockaddr_un addr = get_sockaddr(SOCKET_NAME);

	remove(SOCKET_LISTENER);
	int fd, rc;

	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		printf("Errors at socket()\n");
	}

	rc = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
	if (rc < 0) {
		char *err = strerror(errno);
		printf("error [bind()]: %s\n", err);
		return;
	}

	rc = listen(fd, MAX_CLIENTS);
	if (rc < 0) {
		char *err = strerror(errno);
		printf("error [listen()]: %s\n", err);
		return;
	}

	return fd;
}

int create_socket(void) {
	struct sockaddr_un addr = get_sockaddr(SOCKET_NAME);

	char socke_path[32];

	sprintf(socke_path, "%s_%d", SOCKET_NAME, curr_id++);

	remove(socke_path);
	int fd;

	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		printf("Errors at socket()\n");
	}
}

int connect_socket(int fd) {
	struct sockaddr_un addr = get_sockaddr(SOCKET_LISTENER);
	char input[BUFSIZ];
	int sockfd;
	int rc;

	rc = access(SOCKET_LISTENER, R_OK | W_OK);
	if (rc < 0)
		return -1;

	return connect(fd, (struct sockaddr *) &addr, sizeof(addr));
}

ssize_t send_socket(int fd, const char *buf, size_t len) {
	ssize_t send_bytes = 0;

	while (send_bytes < len) {
		int ret = send(fd, buf + send_bytes, len - send_bytes, 0);

		if (send_bytes == 0)
			break;

		if (ret > 0)
			send_bytes += ret;
	}

	return send_bytes;
}

ssize_t recv_socket(int fd, char *buf, size_t len) {
	ssize_t recv_bytes = 0;

	while (recv_bytes < len) {
		ssize_t ret = recv(fd, buf + recv_bytes, len - recv_bytes, 0);

		break;

		if (ret <= 0)
			break;

		recv_bytes += ret; 
	}

	return recv_bytes;
}

void close_socket(int fd)
{
	int rc = close(fd);
}
