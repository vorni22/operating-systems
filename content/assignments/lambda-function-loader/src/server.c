// SPDX-License-Identifier: BSD-3-Clause

#include <dlfcn.h>
#include <fcntl.h>
#define __USE_XOPEN2K8 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <pthread.h>
#include <sys/un.h>
#include <errno.h>

#include "ipc.h"
#include "server.h"

#ifndef OUTPUT_TEMPLATE
#define OUTPUT_TEMPLATE "../checker/output/out-XXXXXX"
#endif

void init_lib(struct lib *lib) {
	lib->filename = calloc(1, BUFSIZ);

	lib->funcname = calloc(1, BUFSIZ);
	memcpy(lib->funcname, "run", 3);

	lib->libname = calloc(1, BUFSIZ);

	lib->outputfile = calloc(1, BUFSIZ);
}

static int lib_prehooks(struct lib *lib)
{
	/* TODO: Implement lib_prehooks(). */
	char path[strlen(OUTPUT_TEMPLATE) + 1];
	path[strlen(OUTPUT_TEMPLATE)] = 0;
	memcpy(path, OUTPUT_TEMPLATE, strlen(OUTPUT_TEMPLATE));

	int output_file = mkstemp(path);
	lib->output_fd = output_file;

	memcpy(lib->outputfile, path, strlen(path) + 1);
	return 0;
}

static int lib_load(struct lib *lib)
{
	/* TODO: Implement lib_load(). */
	void *handle = dlopen(lib->libname, RTLD_LAZY);
	if (!handle) {
		return -1;
	}

	dlerror();	/* Clear existing errors */

	lib->handle = handle;
	return 0;
}

static int lib_execute(struct lib *lib)
{
	/* TODO: Implement lib_execute(). */

	void *raw_function_ptr = NULL;
	//int output_file = open(lib->outputfile, O_CREAT | O_TRUNC | O_WRONLY, 0644);

	int output_file = lib->output_fd;

	raw_function_ptr = dlsym(lib->handle, lib->funcname);
	char *error = dlerror();

	if (error) {
		//dprintf(lib->output_fd, "%s\n", error);
		return -1;
	}
	dlerror();   /* Clear existing errors */

	/* C has attrocious error handling, so we make a new process for handling exceptions */

	pid_t pid = fork();

	if (pid < 0) {
		fprintf(stderr, "Error creating new process, returning");
		return -1;
	} else if (!pid) {
		//int stdout_copy = dup(STDOUT_FILENO);
		dup2(output_file, STDOUT_FILENO);
		if (lib->filename[0] == 0) {
			void (*function_ptr)(void) = (void (*)(void))raw_function_ptr;  // Cursed line number 1
			(*function_ptr)();
		} else {
			void (*function_ptr)(const char *) = (void (*)(const char *))raw_function_ptr; // Cursed line number 2
			(*function_ptr)(lib->filename);
		}
		close(output_file);
		exit(EXIT_SUCCESS);
		/* Return standard output to it's original fd */
	} else {
		int status;
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status)){
			//dprintf(output_file, "Error : %s could not be executed\n", lib->funcname);
			return -1;
		}
	}
	return 0;
}

static int lib_close(struct lib *lib)
{
	/* TODO: Implement lib_close(). */
	dlclose(lib->handle);

	return 0;
}

static int lib_posthooks(struct lib *lib)
{
	/* TODO: Implement lib_posthooks(). */
	return 0;
}

static int lib_run(struct lib *lib)
{
	int err;

	err = lib_prehooks(lib);
	if (err)
		return err;

	err = lib_load(lib);
	if (err)
		return err;

	err = lib_execute(lib);
	if (err)
		return err;

	err = lib_close(lib);
	if (err)
		return err;

	return lib_posthooks(lib);
}

static int parse_command(const char *buf, char *name, char *func, char *params)
{
	int ret;

	ret = sscanf(buf, "%s %s %s", name, func, params);
	if (ret < 0)
		return -1;

	return ret;
}

void* connection_proc(int socketfd) {
	//printf("Connection created!\n");

	char buf[BUFSIZ];
	int ret;

	ssize_t rec = recv_socket(socketfd, buf, BUFSIZ);
	buf[rec] = 0;
	//printf("[Server]: Recived message: %s\n", buf);

	if (rec <= 0) {
		close_socket(socketfd);
		
		exit(0);
	}

	struct lib lib;
	init_lib(&lib);

	ret = parse_command(buf, lib.libname, lib.funcname, lib.filename);

	if (ret < 0) {
		close_socket(socketfd);
		
		exit(0);
	}

	ret = lib_run(&lib);
	if (ret < 0) {
		dprintf(lib.output_fd, "Error: %s could not be executed.\n", buf);
	}
	send_socket(socketfd, lib.outputfile, strlen(lib.outputfile));

	close_socket(socketfd);

	close(lib.output_fd);

	exit(0);
}

int main(void)
{
	/* TODO: Implement server connection. */
	int ret, listenfd;
	int rc;

	listenfd = create_listener();

	//printf("Listener with fd = %d\n", listenfd);

	while (1) {
		//printf("Waiting for connection\n");
		int connectfd = accept(listenfd, NULL, NULL);
		if (connectfd < 0) {
			char *err = strerror(errno);
			fprintf(stderr, "error [accept()]: %s\n", err);
			return;
		}

		//printf("Connection found!\n");
		
		int pid = fork();

		if (pid < 0) {
			exit(1);
		}
		if (pid == 0) {
			connection_proc(connectfd);
		}

		close(connectfd);
	}

	close_socket(listenfd);

	return 0;
}