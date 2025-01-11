// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"

int curr_id = 0;

struct sockaddr_un get_sockaddr(const char *path) {
	struct sockaddr_un addr;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, strlen(path) + 1, "%s", path);

	return addr;
}

int create_socket(void) {
	struct sockaddr_un addr = get_sockaddr(SOCKET_NAME);

	char socke_path[32];

	sprintf(socke_path, "%s%d", SOCKET_NAME, curr_id++);

	remove(socke_path);

	int rc, listenfd;

	listenfd = socket(PF_UNIX, SOCK_STREAM, 0);
}

int connect_socket(int fd) {
	return connect(fd, NULL, NULL);
}

ssize_t send_socket(int fd, const char *buf, size_t len) {
	size_t send_bytes = 0;

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
	size_t recv_bytes = 0;

	while (recv_bytes < len) {
		int ret = recv(fd, buf, len - recv_bytes, 0);

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
