#ifndef CODIM_ERR_H
#define CODIM_ERR_H

// `error.h` adds error handling utilities.

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define LOG(...) do { \
	fprintf(stderr, "log %s:%d (%s()): ", __FILE__, __LINE__, __func__); \
	fprintf(stderr, __VA_ARGS__); \
	fputc('\n', stderr); \
} while (0)

#define DIE(err, ...) do { \
	LOG(__VA_ARGS__); \
	error_jmp(err); \
} while (0)

#define DIE_ERRNO(err) DIE(err, "%s\n", strerror(errno))

typedef enum Error {
	ERROR_NONE,
	// Fatal allocation error
	ERROR_ALLOC,

	// For errors relating to IO
	ERROR_OUTPUT,
} Error;

void error_jmp(Error err);
Error error_get();

#endif // !CODIM_ERR_H
