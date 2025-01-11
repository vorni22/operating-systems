// SPDX-License-Identifier: BSD-3-Clause

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ipc.h"
#include "server.h"

#ifndef OUTPUT_TEMPLATE
#define OUTPUT_TEMPLATE "../checker/output/out-XXXXXX"
#endif

static int lib_prehooks(struct lib *lib)
{
	/* TODO: Implement lib_prehooks(). */
	return 0;
}

static int lib_load(struct lib *lib)
{
	/* TODO: Implement lib_load(). */
	void *handle = dlopen(lib->libname, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "Dinamic library %s could not be found, closing with error:\n", lib->libname);
		fprintf(stderr, "%s\n", dlerror());
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
	int output_file = open(lib->outputfile, "w");

	raw_function_ptr = dlsym(lib->handle, lib->funcname);
	char *error = dlerror();

	if (error) {
		fprintf(stderr, "%s\n", error);
		return -1;
	}
	dlerror();   /* Clear existing errors */

	/* Redirect output of the function to the file */
	int stdout_copy = dup(STDOUT_FILENO);
	dup2(output_file, STDOUT_FILENO);

	if (lib->filename == NULL) {
		void (*function_ptr)(void) = (void (*)(void))raw_function_ptr;  // Cursed line number 1
		(*function_ptr)();
	} else {
		void (*function_ptr)(const char *) = (void (*)(const char *))raw_function_ptr; // Cursed line number 2
		(*function_ptr)(lib->filename);
	}

	/* Return standard output to it's original fd */
	dup2(stdout_copy, output_file);
	close(stdout_copy);
	close(output_file);

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

int main(void)
{
	/* TODO: Implement server connection. */
	int ret;
	struct lib lib;

	while (1) {
		/* TODO - get message from client */
		/* TODO - parse message with parse_command and populate lib */
		/* TODO - handle request from client */
		ret = lib_run(&lib);
	}

	return 0;
}
